#define F_CPU 16000000UL      
#define F_USB 16000000UL  
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include "VirtualSerial.h" 
#include <util/delay.h> 
 
#define kp  12 
#define ki  23 
#define consigne 15999958UL //Consigne qu'on a trouver aprés le calcule du point de fonctionnement de notre oscillateur
 
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface; 
extern FILE USBSerialStream; 
 
volatile signed int pps2 = 0;         // valeur actuelle du timer (ICR1) 
volatile signed int overflow = 0;     // nombre d'overflow entre deux PPS 
volatile int nOVF = 0;                // copie pour calcul 
volatile signed long f = 0;           // fréquence calculée (nombre de ticks) 
volatile signed int delta = 0;        // différence entre deux captures 
volatile signed char flag = 0;        // indique qu'une nouvelle mesure est prête 
      
 
 
 
//----interrupts---- 
 
ISR (TIMER1_OVF_vect) { 
    overflow++; 
} 
 
ISR (TIMER1_CAPT_vect) { 
 
    pps2 = ICR1; 
    nOVF = overflow; 
    f = (((signed long)nOVF)*65536)+pps2; //Calcule de la frequance en decimale
     
    TCNT1 =0; 
    overflow = 0; 
    flag = 1; 
} 
//----functions---- 
 
void init_PWM () { 
     
    // PD7 = sortie OCR4D 
    DDRD |= (1 << PD7); 
     
    TCCR4D = (1 << PWM4X); 
 
    TCCR4C = (1 << COM4D1) | (1 << PWM4D); 
    TCCR4C &= ~(1 << COM4D0); 
 
    // TOP = 1023 (10 bits) 
    TC4H =  0x03; // Masque sur les 2 bits de poids fort 
    OCR4C =  0xFF; 
 
    // Rapport cyclique = 50% → 512 
    TC4H = 0x01;  
    OCR4D =0xFF; 
 
    // Prescaler = 1 
    TCCR4B = (1 << CS40); 
} 
 
//Fonction pour varier le rapport cyclique dans la boucle fermée
void PWM_rapport_cyclique (short RC) { 
    TC4H = (((RC) & 0x300) >> 8); 
    OCR4D = (RC & 0xFF); 
} 
 
 
void input_capture () { 
 
    DDRD &= ~(1 << PD4); //Entree de de notre 1PPS

 
    TCCR1B |= (1 << ICES1); 
    // sÃ©lection du front montant pour la capture 
 
    TIMSK1 |= (1 << ICIE1) | (1 << TOIE1); 
    // active : 
    // - interruption capture (ICIE1) 
    // - interruption overflow (TOIE1) 
 
    TCCR1B |= (1 << CS10); 
    // prescaler = 1 pour un autre registre
} 
 
 //saturation de la commande (anti-windup) pour eviter divergence 
int sature_commande(long commande) { 
    if (commande > 1023) return 1023; // Max positive ou negative
    if (commande < 0)    return 0;    // Min
    return (int)commande;  
} 
 
 
//-----main----- 
 
int main(void) { 
 
    SetupHardware(); 
    input_capture(); 
    init_PWM(); 
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream); 
    GlobalInterruptEnable(); 
    sei(); //global interrupts too
 
   
    long erreur = 0; //erreur actuelle
    long erreur_old = 0; //erreur dans le PPS d'avant
    long commande_pwm = 0; //la valeur a appliquer au rapport cyclique pour corriger l'erreur
 
while (1) { 
	
    // Execution avec chaque PPS
    if (flag == 1) { 
         
        erreur = consigne - f; 

        commande_pwm = kp * (erreur - erreur_old) + ki * erreur; 
         
        // On accumule la commande pour modifier le point de fonctionnement 
        static long commande_accumulee = 0; 
        commande_accumulee += commande_pwm; 
 
        // Anti-windup
        if (commande_accumulee < -512000) commande_accumulee = -512000; 
        if (commande_accumulee > 511000)  commande_accumulee = 511000; 
 
        erreur_old = erreur; 
 
        // Mise à jour du PWM (Default 512 pour avoir une plage de variation positive our negative) 
        int rapport = 512 + (int)(commande_accumulee / 100); 
        PWM_rapport_cyclique(sature_commande(rapport)); 
 
        // Affichage des données 
        fprintf(&USBSerialStream, "F:%ld | Err:%ld | Cmd:%d\n\r", f, erreur, (int)(commande_accumulee / 100)); 
 
        // On baisse le flag pour attendre le prochain PPS 
        flag = 0; 
    } 
 
    CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface); 
    CDC_Device_USBTask(&VirtualSerial_CDC_Interface); 
    USB_USBTask(); 
    } 
} 

