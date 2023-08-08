/* 
 * File:   Madre/Maestro mainLab.c
 * Author: Christian Alessandro Campos López 21760
 *
 * Created on 5 de agosto de 2023, 07:59 PM
 */

//******************************************************************************
// Palabra de configuración
//******************************************************************************
// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

//******************************************************************************
// Librerías Generales
//******************************************************************************
#include <xc.h>
#include <stdint.h>
#include <pic16f887.h>

//******************************************************************************
// Librerías Propias
//******************************************************************************
#include "I2C.h"
#include "lcd8.h"
//******************************************************************************
// Variables
//******************************************************************************
#define _XTAL_FREQ 8000000     // Frecuencia de oscilador de 8MHz
uint8_t segundos, segundosF=0; 
uint8_t segundos_pasado=1;
char segundosR[3];
char pot1R[3];
uint8_t pot1=0;
uint8_t pot1_pasado=1;
uint8_t bandera=0;
//******************************************************************************
// Prototipos de función
//******************************************************************************
void setup(void);
//******************************************************************************
// Vector de Interrupción
//******************************************************************************

void __interrupt() isr(void) {
}
//******************************************************************************
// Función principal
//******************************************************************************

void main(void) {
    setup(); // Inicializar el microcontrolador

    while (1) {
        
       
        I2C_Master_Start();
        I2C_Master_Write(0x51);
        pot1 = I2C_Master_Read(0);
        I2C_Master_Stop();
        
        // Lectura del RTC DS1340Z-18 (Segundos)
        I2C_Master_Start();
        I2C_Master_Write(0xD0); // Dirección de escritura del RTC
        I2C_Master_Write(0x00); // Registro de segundos en el RTC
        I2C_Master_Stop();

        I2C_Master_Start();
        I2C_Master_Write(0xD1); // Dirección de lectura del RTC
        segundos = I2C_Master_Read(0); // Lee los segundos y envía NACK para finalizar la lectura
        I2C_Master_Stop();
        
        
        // Dividir los bits de 0-3 (unidades) y 4-6 (decenas)
        uint8_t unidades = segundos & 0x0F; // Bits 0-3 (unidades)
        uint8_t decenas = (segundos & 0x70) >> 4; // Bits 4-6 (decenas)
        segundosF=unidades+(10*decenas);

        
        

        
        if (pot1 != pot1_pasado){
            pot1R[0] = (pot1 / 100) + '0'; // Convertir centenas a carácter ASCII
            pot1R[1] = ((pot1 / 10) % 10) + '0'; // Convertir decenas a carácter ASCII
            pot1R[2] = (pot1 % 10) + '0'; // Convertir unidades a carácter ASCII
            pot1_pasado=pot1;
            bandera=1;
       }
        if (segundosF != segundos_pasado){
            segundosR[0] = (segundosF / 100) + '0'; // Convertir centenas a carácter ASCII
            segundosR[1] = ((segundosF / 10) % 10) + '0'; // Convertir decenas a carácter ASCII
            segundosR[2] = (segundosF % 10) + '0'; // Convertir unidades a carácter ASCII
            segundos_pasado = segundosF;
            bandera=1;
       }
        if (bandera==1){
            Lcd_Clear();
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("Segundos POT1");
            __delay_us(5);
            Lcd_Set_Cursor(2,1);
            Lcd_Write_Char(segundosR[0]); // Mostrar centenas
            Lcd_Write_Char(segundosR[1]); // Mostrar decenas
            Lcd_Write_Char(segundosR[2]); // Mostrar unidades
            Lcd_Write_String(" ");
            Lcd_Write_Char(pot1R[0]); // Mostrar centenas
            Lcd_Write_Char(pot1R[1]); // Mostrar decenas
            Lcd_Write_Char(pot1R[2]); // Mostrar unidades
            bandera=0;       
       }
        
        __delay_ms(200);
    }
    
}
//******************************************************************************
// Funciones secundarias
//******************************************************************************

void setup(void) {
    // Configuración de los puertos
    ANSEL = 0;
    ANSELH = 0;
    TRISB = 0;
    TRISD = 0;
    PORTB = 0;
    PORTD = 0;
    
    //Oscilador a 8MHz
    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 1;

    //Configuraci�n LCD
    Lcd_Init();
    //Inicializar Comuncación I2C
    I2C_Master_Init(100000);
}




