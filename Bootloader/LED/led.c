#include <avr/io.h> //E/S ex PORTB 
#define F_CPU 16000000UL
#define F_USB 16000000UL
#include <util/delay.h>
#include "VirtualSerial.h"

int main(void){
	SetupHardware();
	MCUCR|=(1<<JTD);
	MCUCR|=(1<<JTD);

	DDRF |= (1 << PORTF1)|(1 << PORTF4)|(1 << PORTF5)|(1 << PORTF6);
	PORTF |= (1 << PORTF1)|(1 << PORTF4)|(1 << PORTF5)|(1 << PORTF6);
	
	while (1){
		//PORTF^=1<<PORTF1;
				PORTF^=1<<PORTF1;PORTF^=1<<PORTF4;PORTF^=1<<PORTF5;PORTF^=1<<PORTF6;

		_delay_ms(100);
	}
	return 0;
}
