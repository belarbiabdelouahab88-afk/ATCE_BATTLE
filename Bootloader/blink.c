#include <avr/io.h> //E/S ex PORTB 
#define F_CPU 16000000UL
#define F_USB 16000000UL
#include <util/delay.h>
#include "VirtualSerial.h"

int main(void){
  SetupHardware();
  DDRF |=1<<PORTF1;
  PORTF |= 1<<PORTF1;

  while (1){
    /* Clignotement des LEDS */
  PORTF^=1<<PORTF1;
  _delay_ms(500);
  }
  return 0;
}
