#define F_CPU 16000000UL     
#define F_USB 16000000UL 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "VirtualSerial.h"
#include <util/delay.h>

#define kp  54
#define ki  63
#define consigne 16000000UL

extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;
extern FILE USBSerialStream;

volatile signed int pps2 = 0;        // valeur actuelle du timer (ICR1)
volatile signed int overflow = 0;     // nombre d'overflow entre deux PPS
volatile int nOVF = 0;         // copie pour calcul
volatile signed long f = 0;           // fréquence calculée (nombre de ticks)
volatile signed int delta = 0;        // différence entre deux captures
volatile signed char flag = 0;        // indique qu'une nouvelle mesure est prête
     

//uint8_t flag_consigne = 0;


//----interrupts----

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
//----functions----

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


void PWM_rapport_cyclique (short RC) {
    TC4H = (((RC) & 0x300) >> 8);
    OCR4D = (RC & 0xFF);
}


void input_capture () {

    DDRD &= ~(1 << PD4);
    // PD4 en entrÃ©e â†’ reÃ§oit le signal 1 PPS du GPS

    TCCR1B |= (1 << ICES1);
    // sÃ©lection du front montant pour la capture

    TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);
    // active :
    // - interruption capture (ICIE1)
    // - interruption overflow (TOIE1)

    TCCR1B |= (1 << CS10);
    // prescaler = 1 â†’ timer Ã  16 MHz 
}

void sature_integrale(float *integrale_val) {
    if (*integrale_val > 500.0) *integrale_val = 500.0;   
    if (*integrale_val < -500.0) *integrale_val = -500.0;
}


int sature_commande(long commande) {
    if (commande > 1023) return 1023; // Ceiling
    if (commande < 0)    return 0;    // Floor
    return (int)commande; 
}


//-----main-----

int main(void) {

    SetupHardware();
    input_capture();
    init_PWM();
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
    GlobalInterruptEnable();
    sei(); //global interrupts

  
    long erreur = 0;
    long erreur_old = 0;
    long int commande_pwm = 0;
    //float integrale = 0;

    while (1) {
		
    //if (flag_consigne==0){
      //  if (flag==1) {
            //fprintf(&USBSerialStream, "%u,%u,%ld\n\r", nOVF,pps2, f);                     
           // flag = 0;
      //  }
  //  }
 //   else{
		
    //------Regulation------
    
    erreur= consigne - f;
    commande_pwm=kp *( erreur - erreur_old) + ki * (erreur+commande_pwm);
    
    //limitation du changement de la commande +-50*100
    if (commande_pwm < -5000) {
		commande_pwm = -5000;
	}
    if (commande_pwm > 5000) {
		commande_pwm = 5000;
	}

    //integrale+=erreur;
    //sature_integrale(&integrale);

    
    //sature_commande(commande_pwm);

    erreur_old=erreur;

    //512 because we want the regulation to have room to modify the value either up or down

    PWM_rapport_cyclique(512 + commande_pwm/1000);

    fprintf(&USBSerialStream, "%X,%lX,%lX\n\r", nOVF, erreur, commande_pwm/1000);


  //  }

        CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        USB_USBTask();
    }
}
