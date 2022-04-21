/*
 * File:   Lab8.c
 * Author: Josea
 *
 * Created on 20 de abril de 2022, 05:23 PM
 */



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

#include <xc.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 4000000


/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
int valor = 0, selectorDSP = 0, DSP1 = 0, DSP2 = 0, DSP3 = 0, unidades = 0, decenas = 0, centenas = 0, temp = 0;

uint8_t DISPLAY[10]= {0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111};
/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/


void setup(void);
void DSPsetup(void);
void separacion(void);
void multiplexado(void);
/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.ADIF){              // Fue interrupción del ADC?
        if(ADCON0bits.CHS == 0){    // Verificamos sea AN0 el canal seleccionado
            PORTC = ADRESH;         // Mostramos ADRESH en PORTC
        }
      
        else if (ADCON0bits.CHS == 1){
            multiplexado();
        }
        PIR1bits.ADIF = 0;          // Limpiamos bandera de interrupción
    }
    
     if(T0IF){                           // Interrupcion del timer0
        if(selectorDSP < 4) 
           selectorDSP++;               // Incremento del selector para el multiplexado
        else
            selectorDSP = 0;            // solo tenemos 3 displays
        
        TMR0 = 249;
        INTCONbits.T0IF = 0;            // Reiniciamos el timer0
     
    }              
    
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();
    while(1){
        if(ADCON0bits.GO == 0){             // No hay proceso de conversion
            if(ADCON0bits.CHS == 0b0000)    
                ADCON0bits.CHS = 0b0001;    // Cambio de canal
            else if(ADCON0bits.CHS == 0b0001)
                ADCON0bits.CHS = 0b0000;    // Cambio de canal
            __delay_us(40);                 // Tiempo de adquisición            
            ADCON0bits.GO = 1;              // Iniciamos proceso de conversión
        }
        
       
        
    }
    return;
}

void multiplexado(void){
     //multiplexado
        separacion();
        DSPsetup();
        switch(selectorDSP)
    {
        case 0:
            PORTE = 0;
            break;
        case 1:
            PORTE = 0;
            PORTD = DSP1;
            PORTEbits.RE0 = 1;
            break;
        case 2:
            PORTE = 0;
            PORTD = DSP2;
            PORTEbits.RE1 = 1;
            break;
        case 3:
            PORTE = 0;
            PORTD = DSP3;
            PORTEbits.RE2 = 1;
            break;
    }
}


void separacion(void){
    valor = 1.96*(ADRESH);
    
    temp = valor;
    
                    // vemos si sí tenemos que ver cuantas centenas hay
        centenas = temp/100;        // vemos cuantas centenas hay
        temp = temp%100;            // movemos el residuo a temp
   
        decenas = temp/10;
        temp = temp%10;
   
        unidades = temp;
    
}

void DSPsetup(void){ 
        DSP1= DISPLAY[centenas];    //movemos cada valor al display correspondiente ya codificado para los 7 segmentos usando un array.
        DSP2= DISPLAY[decenas];
        DSP3= DISPLAY[unidades];
    return;
}


/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    ANSELH = 0;         // I/O digitales)
    
    ANSEL = 0b00000011; // AN0, AN1 y AN2 como entrada analógica
    TRISA = 0b00000011; // AN0, AN1  como entrada
    PORTA = 0; 
    
    TRISC = 0;
    PORTC = 0;
    
    TRISD = 0;
    PORTD = 0;
    
    TRISE = 0;
    PORTE = 0;
    
    // Configuración reloj interno
    OSCCONbits.IRCF = 0b0110;   // 4MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuraciones del ADC
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    
    ADCON1bits.VCFG0 = 0;       // VDD *Referencias internas
    ADCON1bits.VCFG1 = 1;       // VSS
    
    ADCON0bits.CHS = 0b0000;    // Seleccionamos AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitamos modulo ADC
    __delay_us(40);
    
    // Configuracion de interrupciones
    PIR1bits.ADIF = 0;          // Limpiamos bandera de int. ADC
    PIE1bits.ADIE = 1;          // Habilitamos int. de ADC
    INTCONbits.PEIE = 1;        // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;         // Habilitamos int. globales
    
    //Timer 0 a 1ms
    OPTION_REGbits.T0CS = 0;  // bit 5  TMR0
    OPTION_REGbits.T0SE = 0;  // bit 4 TMR0 
    OPTION_REGbits.PSA = 0;   // bit 3  
    OPTION_REGbits.PS2 = 0;   // bits 2-0  
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
}
