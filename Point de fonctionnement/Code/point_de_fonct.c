//Variation du rapport cyclique sur le signal de frequence pour visualiser le point de fonctionnement 
#define F_CPU 16000000UL     
#define F_USB 16000000UL     
#include <avr/io.h>
#include <avr/interrupt.h>
#include "VirtualSerial.h"
#include <util/delay.h>

extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;
extern FILE USBSerialStream;

volatile signed int pps2 = 0;        // valeur actuelle du timer (ICR1)
volatile signed int overflow = 0;     // nombre d'overflow entre deux PPS
volatile signed int nOVF = 0;         // copie pour calcul
volatile signed long f = 0;           // fréquence calculée (nombre de ticks)
volatile signed int delta = 0;        // différence entre deux captures
volatile signed char flag = 0;        // indique qu'une nouvelle mesure est prête

ISR (TIMER1_OVF_vect) {
    overflow++;
}


ISR (TIMER1_CAPT_vect) {

    pps2 = ICR1;
    nOVF = overflow;
    f = (((signed long)nOVF)*65536)+pps2;
    
    TCNT1 =0;
    overflow = 0;
    flag = 1;
}

void input_capture () {

    DDRD &= ~(1 << PD4);
    // PD4 en entrée → reçoit le signal 1 PPS du GPS

    TCCR1B |= (1 << ICES1);
    // sélection du front montant pour la capture

    TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);
    // active :
    // - interruption capture (ICIE1)
    // - interruption overflow (TOIE1)

    TCCR1B |= (1 << CS10);
    // prescaler = 1 
}


void init_PWM () {
	
    // PD7 = sortie OC4D
    DDRD |= (1 << PD7);
    
	TCCR4D = (1 << PWM4X);

    TCCR4C = (1 << COM4D1) | (1 << PWM4D);
    TCCR4C &= ~(1 << COM4D0);

    // TOP = 1023 (10 bits)
    TC4H =  0x03; // Masque sur les 2 bits de poids fort
	OCR4C =  0xFF;

    // Duty cycle = 50% → 512
    TC4H = 0x01; 
	OCR4D =0xFF;

    // Prescaler = 1
    TCCR4B = (1 << CS40);
}



void PWM_rapport_cyclique ( short RC) {
	TC4H = (((RC) & 0x300) >> 8);
	OCR4D = (RC & 0xFF);
}



int main(void) {

    SetupHardware();
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
    input_capture();
    init_PWM();
    GlobalInterruptEnable();
    sei(); 
    uint16_t rapport_actuel = 0;

    while (1) {
        if (flag==1) {
            fprintf(&USBSerialStream, "%u,%u,%ld\n\r", nOVF,pps2, f);                     
            flag = 0;
     
            rapport_actuel += 10;

            // Recommence à 0 si on dépasse la valeur TOP (1023)
            if (rapport_actuel > 1023) {
                rapport_actuel = 0;
            }

            // Met à jour le registre du Timer 4
            PWM_rapport_cyclique(rapport_actuel);
        }
        CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        USB_USBTask();
    }
}
