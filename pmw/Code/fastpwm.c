//Generation de la PWM du timer4 d'une resolution de 10 bits sur la sortie PD7 de l'atemega32u4
#define F_CPU 16000000UL
#define F_USB 16000000UL
#include <avr/io.h>
#include "VirtualSerial.h"
#include <util/delay.h>

extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;
extern FILE USBSerialStream;

void init_PWM () {
	
    // PD7 = sortie OC4D
    DDRD |= (1 << PD7);
    
    // Activation du mode PWM_Fast
	TCCR4D = (1 << PWM4X); 

	//Activation de la PWM
    TCCR4C = (1 << COM4D1) | (1 << PWM4D);
    TCCR4C &= ~(1 << COM4D0);

    // Resolution de 1023
    TC4H =  0x03;  // Masque sur les 2 bits de poids fort
	OCR4C =  0xFF;

    // Rapport cyclique
    TC4H = 0x01; 
	OCR4D =0xFF;

    // Prescaler = 1
    TCCR4B = (1 << CS40);
}

int main(void) {
    init_PWM();
    SetupHardware();
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
	GlobalInterruptEnable();
    while (1) {	
		
        }
}
