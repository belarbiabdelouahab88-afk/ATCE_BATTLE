//Declenchement de l'interruption capture edge select pour detecter le signal 1PPS sur les fronts montants du Timer1
#define F_CPU 16000000UL     
#define F_USB 16000000UL     
#include <avr/io.h>
#include <avr/interrupt.h>
#include "VirtualSerial.h"
#include <util/delay.h>

extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;
extern FILE USBSerialStream;

volatile signed int PPS2 = 0;        // valeur actuelle du timer (ICR1)
volatile signed int overflow = 0;     // nombre d'overflow entre deux PPS
volatile signed int nOVF = 0;         // copie pour calcul
volatile signed long f = 0;           // fréquence calculée (nombre de ticks)
volatile signed int delta = 0;        // différence entre deux captures
volatile signed char flag = 0;        // indique qu'une nouvelle mesure est prête

ISR (TIMER1_OVF_vect) {
    overflow++;
}


ISR (TIMER1_CAPT_vect) {

    PPS2 = ICR1; // Stocke la valeur du compteur au moment de la détection 
    nOVF = overflow;
    f = (((signed long)nOVF)*65536)+PPS2; // Le calcul de la fréquence mesuré 

    //Remise a 0 du compteur et de l'overflow pour préparer la prochaine mesure
    TCNT1 =0; 
    overflow = 0; 
    flag = 1; // lève un drapeau pour indiquer qu'une mesure a été effectué 
}

void input_capture () {
    // PD4 en entrée → reçoit le signal 1 PPS du GPS
    DDRD &= ~(1 << PD4);
    
    // sélection du front montant pour la capture
    TCCR1B |= (1 << ICES1);
    

    // activation de :
    // - interruption capture (ICIE1)
    // - interruption overflow (TOIE1)
    TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);
    
    // prescaler = 1
    TCCR1B |= (1 << CS10);
     
}

int main(void) {

    SetupHardware();
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
    input_capture();
    GlobalInterruptEnable();
    sei(); 

    while (1) {
        if (flag==1) {
            fprintf(&USBSerialStream, "%u,%u,%ld\n\r", nOVF,PPS2, f);   // Communication via le port Série                  
            flag = 0;
        }      
        CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        USB_USBTask();
    }
}
