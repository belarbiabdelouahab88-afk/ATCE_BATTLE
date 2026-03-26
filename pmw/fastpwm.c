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
    
    // Mode PWM : Phase & Frequency Correct (WGM40 = 1)
    TCCR4D = (1 << WGM40);

    // Activer OC4D + PWM4D
    TCCR4C = (1 << COM4D1) | (1 << PWM4D);

    // TOP = 1023 (10 bits)
    TC4H =  0x03; // Masque sur les 2 bits de poids fort
    OCR4C =  0xFF;

    // Duty cycle = 50% → 512
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
