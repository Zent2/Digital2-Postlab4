/* 
 * File:   mainPostLab.c
 * Author: Christian Alessandro Campos López 21760
 *
 * Created on 5 de agosto de 2023, 10:22 PM
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
#include "char2ASC.h"
#include "ioc_init.h"
//******************************************************************************
// Variables
//******************************************************************************
#define _XTAL_FREQ 8000000     // Frecuencia de oscilador de 8MHz
uint8_t segundos, segundosF=0, segundos_pasado=1;
uint8_t minutos, minutosF=0, minutos_pasado=1;
uint8_t horaF=0, hora_pasado=1;
uint8_t diaF=0, dia_pasado=1;
uint8_t mesF=0, mes_pasado=1;
uint8_t yearF=0, year_pasado=1;
char segundosR[3];
char minutosR[3];
char horaR[3];
char diaR[3];
char mesR[3];
char yearR[3];
char pot1R[3];
uint8_t pot1=0;
uint8_t pot1_pasado=1;
uint8_t bandera=0,bandera2=0;
uint8_t modo=1;
int8_t CONT;
uint8_t seleccionador, seleccionador_pasado=5;
//******************************************************************************
// Prototipos de función
//******************************************************************************
void setup(void);
char lectura_RTC(uint8_t Registro, uint8_t ultimos_bits);
void escritura_RTC(uint8_t Registro, uint8_t valor, uint8_t ultimos_bits);
//******************************************************************************
// Vector de Interrupción
//******************************************************************************

void __interrupt() isr(void) {
    if (INTCONbits.RBIF) {
        
        // Verificar si el pushbutton de seleccionador
        if (!PORTBbits.RB7) {
            CONT=0; //Al cambiar la variable, se resetea CONT
            seleccionador++; //Selecciona la variable de la fecha y hora
                            //Que se desea cambiar
            
        }

        // Verificar si el pushbutton de suma fue presionado
        if (!PORTBbits.RB5) {
            CONT++; // CONT es el valor que se escribira en el RTC
            
        }
        // Verificar si el pushbutton de resta fue presionado
        if (!PORTBbits.RB3) {
            CONT--;
        }

        INTCONbits.RBIF = 0; // Limpiar la bandera de interrupción del puerto B
    }
}
//******************************************************************************
// Función principal
//******************************************************************************

void main(void) {
    setup(); // Inicializar el microcontrolador

    while (1) {
        while (modo==1){ //Este modo es el modo de escritura al RTC
            if (bandera2==0){
                //Solo mostramos que estamos en modo de escritura
                Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Write_String("MODO");
                Lcd_Clear();
                Lcd_Set_Cursor(2,1);
                Lcd_Write_String("ESCRITURA"); 
                bandera2=1;
                __delay_ms(500);
            }
            if (bandera2==1){
                //Ya aquí viene lo bueno xd 
                //Case para el seleccionador (ver interrupciones)
                switch (seleccionador){ 
                case 0:
                    segundosF=CONT; //Se escribe CONT en la variable
                                    // Esto se repite en todas las variables  
                    if (CONT>59){ //Segundos se limita a 0-59
                        CONT=0;
                    }
                    if (CONT<0){
                        CONT=59;
                    }
                break;
                case 1:
                    minutosF=CONT;
                    if (CONT>59){//Minutos se limita a 0-59
                        CONT=0;
                    }
                    if (CONT<0){
                        CONT=59;
                    }
                break;
                case 2:
                    horaF=CONT;
                    if (CONT>23){ //Hora se limita a 0-23
                        CONT=0;
                    }
                    if (CONT<0){ 
                        CONT=23;
                    }
                break;
                case 3:
                    yearF=CONT; 
                    if (CONT>99){ //Año se limita a 0-99
                        CONT=0;
                    }
                    if (CONT<0){
                        CONT=99;
                    }
                break;
                case 4:
                    mesF=CONT;
                    if (CONT>12){ //Mes se limita a 0-12
                        CONT=0;
                    }
                    if (CONT<0){
                        CONT=12;
                    }
                break;
                case 5:
                    diaF=CONT; //Dia depende del año y el mes
                    //Primero ver si es mes de 0-31 días
                    if (mesF==1 || mesF==3 || mesF==5 || mesF==7 || mesF==8 || mesF==10|| mesF==12){
                        if (CONT>31){
                            CONT=0;
                        }
                        if (CONT<0){
                            CONT=31;
                        }
                    }
                    //Después ver si es mes de 0-30 días
                    if (mesF==4 || mesF==6 || mesF==9 || mesF==11){
                        if (CONT>30){
                            CONT=0;
                        }
                        if (CONT<0){
                            CONT=30;
                        }
                    }
                    //Sino, es febrero, pero si es febrero...
                    if (mesF==2){
                        // Verificar si el año es bisiesto (0-29)
                        uint8_t esBisiesto = 0;
                        if ((yearF % 4 == 0 && yearF % 100 != 0) || yearF % 400 == 0) {
                            esBisiesto = 1;
                        }
                        if (esBisiesto) {
                            if (CONT > 29) {
                                CONT = 0;
                            }
                            if (CONT < 0) {
                                CONT = 29;
                            }
                        } 
                        //Sino es bisiesto entonces (0-28)
                        if (!esBisiesto){
                            if (CONT > 28) {
                                CONT = 0;
                            }
                            if (CONT < 0) {
                                CONT = 28;
                            }
                        }  
                    }
                    break;
                    }
                
                }
                
                //Se verifica algún cambio en cualquiera de las variables
                //Así no se está siempre reescribiendo la LCD solo si hay cambio
                if (segundosF != segundos_pasado){
                    convertir_a_ascii(segundosF, segundosR); 
                    segundos_pasado = segundosF;
                    bandera=1;//Esta es la bandera para reescribir la LCD
                }

                if (minutosF != minutos_pasado) {
                    convertir_a_ascii(minutosF, minutosR);
                    minutos_pasado = minutosF;
                    bandera = 1;
                }

                if (horaF != hora_pasado) {
                    convertir_a_ascii(horaF, horaR);
                    hora_pasado = horaF;
                    bandera = 1;
                }

                if (diaF != dia_pasado) {
                    convertir_a_ascii(diaF, diaR);
                    dia_pasado = diaF;
                    bandera = 1;
                }

                if (mesF != mes_pasado) {
                    convertir_a_ascii(mesF, mesR);
                    mes_pasado = mesF;
                    bandera = 1;
                }

                if (yearF != year_pasado) {
                    convertir_a_ascii(yearF, yearR);
                    year_pasado = yearF;
                    bandera = 1;
                }
                if (seleccionador != seleccionador_pasado){
                    seleccionador_pasado=seleccionador;
                    bandera=1;
                }
        
                if (bandera==1){ //Se reescribe la LCD
                    Lcd_Clear();
                    Lcd_Set_Cursor(1,1);
                    Lcd_Write_String("Cambiar"); //Solo muestra que se cambia
                    Lcd_Write_String(" ");
            
                    Lcd_Write_Char(horaR[1]); // Mostrar decenas
                    Lcd_Write_Char(horaR[2]); // Mostrar unidades
                    Lcd_Write_String(":");
                    Lcd_Write_Char(minutosR[1]); // Mostrar decenas
                    Lcd_Write_Char(minutosR[2]); // Mostrar unidades
                    Lcd_Write_String(":");
                    Lcd_Write_Char(segundosR[1]); // Mostrar decenas
                    Lcd_Write_Char(segundosR[2]); // Mostrar unidades
                    __delay_us(5);
                    Lcd_Set_Cursor(2,1);
                    switch (seleccionador) { 
                        //Dependiendo del seleccionador, se muestra la variable
                        //A modificar
                        case 0:
                            Lcd_Write_String("SEG.");
                        break;

                        case 1:
                            Lcd_Write_String("MIN.");
                        break;

                        case 2:
                            Lcd_Write_String("HORA");
                        break;

                        case 3:
                            Lcd_Write_String("YEAR");
                        break;

                        case 4:
                            Lcd_Write_String("MES ");
                        break;

                        case 5:
                            Lcd_Write_String("DIA ");
                        break;
                    }
                    Lcd_Write_String("  ");
                    Lcd_Write_Char(diaR[1]); // Mostrar decenas
                    Lcd_Write_Char(diaR[2]); // Mostrar unidades
                    Lcd_Write_String("/");
                    Lcd_Write_Char(mesR[1]); // Mostrar decenas
                    Lcd_Write_Char(mesR[2]); // Mostrar unidades
                    Lcd_Write_String("/");
                    Lcd_Write_String("20");
                    Lcd_Write_Char(yearR[1]); // Mostrar decenas
                    Lcd_Write_Char(yearR[2]); // Mostrar unidades
            
                    bandera=0;       
                }
            if (seleccionador>5){ 
                //Al dar el último click en el PB se escriben los nuevos valores
                //En la RTC
                escritura_RTC(0x00, segundosF, 0x70);
                escritura_RTC(0x01, minutosF, 0x70);
                escritura_RTC(0x02, horaF, 0x30);
                escritura_RTC(0x04, diaF, 0x30);
                escritura_RTC(0x05, mesF, 0x10);
                escritura_RTC(0x06, yearF, 0xF0);
                seleccionador=0;
                modo=0;
            }    
        }
        //Ahora muestra los valores actuales de la RTC
        
        INTCONbits.RBIE=0; //Aquí ya no funcionan los botones
        //Se lee el POT del PIC esclaso/hijo
        I2C_Master_Start();
        I2C_Master_Write(0x51);
        pot1 = I2C_Master_Read(0);
        I2C_Master_Stop();
        //Se leen los valores de la RTC
        segundosF = lectura_RTC(0x00, 0x70);
        minutosF = lectura_RTC(0x01, 0x70);
        horaF = lectura_RTC(0x02, 0x30); // Registro de Hora (0x02) y últimos_bits (0x30)
        diaF = lectura_RTC(0x04, 0x30); // Registro de Día del mes (0x04) y últimos_bits (0x30)
        mesF = lectura_RTC(0x05, 0x10); // Registro de Mes (0x05) y últimos_bits (0x10)
        yearF = lectura_RTC(0x06, 0xF0); // Registro de Año (0x06) y últimos_bits (0xF0)
        //Mismo código para no estar siempre reescribiendo la LCD
        if (pot1 != pot1_pasado){
            convertir_a_ascii(pot1, pot1R);
            pot1_pasado=pot1;
            bandera=1;
       }
        if (segundosF != segundos_pasado){
            convertir_a_ascii(segundosF, segundosR);
            segundos_pasado = segundosF;
            bandera=1;
       }

        if (minutosF != minutos_pasado) {
            convertir_a_ascii(minutosF, minutosR);
            minutos_pasado = minutosF;
            bandera = 1;
        }

        if (horaF != hora_pasado) {
            convertir_a_ascii(horaF, horaR);
            hora_pasado = horaF;
            bandera = 1;
        }

        if (diaF != dia_pasado) {
            convertir_a_ascii(diaF, diaR);
            dia_pasado = diaF;
            bandera = 1;
        }

        if (mesF != mes_pasado) {
            convertir_a_ascii(mesF, mesR);
            mes_pasado = mesF;
            bandera = 1;
        }

        if (yearF != year_pasado) {
            convertir_a_ascii(yearF, yearR);
            year_pasado = yearF;
            // Realizar las acciones necesarias con anoR (por ejemplo, enviar a una pantalla)
            bandera = 1;
        }
        
        if (bandera==1){
            Lcd_Clear();
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("POT1");
            Lcd_Write_String(" ");
            
            Lcd_Write_Char(horaR[1]); // Mostrar decenas
            Lcd_Write_Char(horaR[2]); // Mostrar unidades
            Lcd_Write_String(":");
            Lcd_Write_Char(minutosR[1]); // Mostrar decenas
            Lcd_Write_Char(minutosR[2]); // Mostrar unidades
            Lcd_Write_String(":");
            Lcd_Write_Char(segundosR[1]); // Mostrar decenas
            Lcd_Write_Char(segundosR[2]); // Mostrar unidades
            __delay_us(5);
            Lcd_Set_Cursor(2,1);
            Lcd_Write_Char(pot1R[0]); // Mostrar centenas
            Lcd_Write_Char(pot1R[1]); // Mostrar decenas
            Lcd_Write_Char(pot1R[2]); // Mostrar unidades
            Lcd_Write_String(" ");
            
            Lcd_Write_Char(diaR[1]); // Mostrar decenas
            Lcd_Write_Char(diaR[2]); // Mostrar unidades
            Lcd_Write_String("/");
            Lcd_Write_Char(mesR[1]); // Mostrar decenas
            Lcd_Write_Char(mesR[2]); // Mostrar unidades
            Lcd_Write_String("/");
            Lcd_Write_String("20");
            Lcd_Write_Char(yearR[1]); // Mostrar decenas
            Lcd_Write_Char(yearR[2]); // Mostrar unidades
            
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
    TRISE=0;
    PORTB = 0;
    PORTD = 0;
    //Push-Buttons
    ioc_init(7);
    ioc_init(5);
    ioc_init(3);
    
    //Oscilador a 8MHz
    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 1;

    //Configuraci n LCD
    Lcd_Init();
    //Inicializar Comuncación I2C
    I2C_Master_Init(100000);
}

char lectura_RTC(uint8_t Registro, uint8_t ultimos_bits) {
    char valor;
    I2C_Master_Start();
    I2C_Master_Write(0xD0); // Dirección de escritura del RTC
    I2C_Master_Write(Registro); // Registro específico en el RTC
    I2C_Master_Stop();
    
    I2C_Master_Start();
    I2C_Master_Write(0xD1); // Dirección de lectura del RTC
    valor = I2C_Master_Read(0); // Lee el valor y envía NACK para finalizar la lectura
    I2C_Master_Stop();
    
    // Dividir los bits de 0-3 (unidades) y los últimos_bits (decenas)
    uint8_t unidades = valor & 0x0F; // Bits 0-3 (unidades)
    uint8_t decenas = (valor & ultimos_bits) >> 4; // Bits 4-7 (decenas)
    return unidades + (10 * decenas);
}
void escritura_RTC(uint8_t Registro, uint8_t valor, uint8_t ultimos_bits) {
    uint8_t decenas = valor / 10;
    uint8_t unidades = valor % 10;
    uint8_t data = (decenas << 4) | (unidades & 0x0F); // Combinar decenas y unidades en un byte
    
    I2C_Master_Start();
    I2C_Master_Write(0xD0); // Dirección de escritura del RTC
    I2C_Master_Write(Registro); // Registro específico en el RTC
    I2C_Master_Write(data); // Valor que se va a escribir en el registro
    I2C_Master_Stop();
    
    // Si es necesario, dividir los últimos_bits y enviarlos a un registro adicional
    if (ultimos_bits != 0) {
        I2C_Master_Start();
        I2C_Master_Write(0xD0); // Dirección de escritura del RTC
        I2C_Master_Write(Registro + 1); // Registro adicional en el RTC para los últimos bits
        I2C_Master_Write(ultimos_bits); // Escribir los últimos bits en el registro adicional
        I2C_Master_Stop();
    }
}
