#include <avr/io.h> //E/S ex PORTB 
#define F_CPU 16000000UL
#include <util/delay.h>

int main(void){

	MCUCR|=(1<<JTD);
	MCUCR|=(1<<JTD);

	DDRF |= (1 << PF1)|(1 << PF4)|(1 << PF5)|(1 << PF6);
	PORTF |= (1 << PF1)|(1 << PF4)|(1 << PF5)|(1 << PF6);
	
	while (1){
		PORTF^=1<<PORTF1;PORTF^=1<<PORTF4;PORTF^=1<<PORTF5;PORTF^=1<<PORTF6;
		_delay_ms(500);
	}
	return 0;
}
