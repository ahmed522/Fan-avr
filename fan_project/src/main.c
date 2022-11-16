/*
 * main.c
 *
 *  Created on: ??þ/??þ/????
 *      Author: Ahmed Abdelaal
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
void PWM_TIMER_INIT(float);
void TIMER2_INIT(unsigned char);
void STOP_FAN();
int timer = 0, sec = 0, min = 0, clk_count = 0, on_Flag = 0;
enum Level {
	off, level1, level2, level3
} level;
ISR(TIMER2_COMP_vect) {
	++clk_count;
	if (clk_count == 5) {
		clk_count = 0;
		++sec;
		if (sec == 60) {
			sec = 0;
			++min;
			if (min == 60) {
				min = 0;
				--timer;
				if (!timer) {
					on_Flag = 0;
					level = off;
					PWM_TIMER_INIT(0);
					PORTC &= ~(1 << PC6);
					TCCR2 &= ~((1 << CS21) | (1 << CS22) | (1 << CS20));
				}
			}

		}
	}
}
int main() {
	DDRD |= (1 << PD1);
	DDRC = 0xFF;
	DDRA &= ~((1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3) | (1 << PA4));
	SREG |= (1 << 7);
	TIMSK |= (1 << OCIE2);
	PWM_TIMER_INIT(0);
	TIMER2_INIT(195);
	TCCR2 &= ~((1 << CS21) | (1 << CS22) | (1 << CS20));
	level = off;
	int timer_pressed = 0, pressed = 0;
	while (1) {
//=====================7-seg==============
		PORTC &= ~(1 << PC4);
		PORTC |= (1 << PC5);
		PORTC = (PORTC & 0xF0) | timer;
		_delay_ms(0.005);
		//==================
		PORTC &= ~(1 << PC5);
		PORTC |= (1 << PC4);
		PORTC = (PORTC & 0xF0) | level;
		_delay_ms(0.005);
//=====================7-seg==============
//==============on indicator led==========
		if (on_Flag) {
			PORTD |= (1 << PD1);
		} else {
			TCCR2 &= ~((1 << CS21) | (1 << CS22) | (1 << CS20));
			timer=0;
			PORTD &= ~(1 << PD1);
		}
//==============on indicator led==========
//==============(start/stop) btn==========
		if ((PINA & 0x10)) {
			_delay_ms(10);
			if ((PINA & 0x10) && !(pressed)) {
				STOP_FAN();
				level = off;
				pressed = 1;
			} else if ((PINA & 0x10) && (pressed)) {
				//do nothing
			}
		} else {
			pressed = 0;
		}

//==============(start/stop) btn==========
//============speed level buttons=========
		if ((PINA & 0x01) && on_Flag) {
			_delay_ms(10);
			if (PINA & 0x01) {
				level = level1;
				PWM_TIMER_INIT(33.33);
			}
			//===================
		} else if ((PINA & 0x02) && on_Flag) {
			_delay_ms(10);
			if (PINA & 0x02) {
				level = level2;
				PWM_TIMER_INIT(66.66);
			}
			//===================
		} else if ((PINA & 0x04) && on_Flag) {
			_delay_ms(10);
			if (PINA & 0x04) {
				level = level3;
				PWM_TIMER_INIT(100);
			}
		}
//============speed level buttons=========
//================timer button============
		if (on_Flag){
			if ((PINA & (1 << PA3)) && !(timer_pressed)) {
				++timer;
				TCNT2 = 0;
				TCCR2 |= ((1 << CS21) | (1 << CS22) | (1 << CS20));
				if (timer > 4) {
					timer = 0;
					TCCR2 &= ~((1 << CS21) | (1 << CS22) | (1 << CS20));
				}
				timer_pressed = 1;
			} else if ((PINA & (1 << PA3)) && (timer_pressed)) {

			} else {
				timer_pressed = 0;
			}
		}
//================timer button============
	}
	return 0;
}
void PWM_TIMER_INIT(float duty_Cycle) {
	unsigned char compare_Val = (unsigned char) duty_Cycle * 255 / 100;
	TCNT0 = 0;
	OCR0 = compare_Val;
	DDRB |= (1 << PB3);
	TCCR0 |= (1 << WGM01) | (1 << WGM00) | (1 << COM01) | (1 << CS02)
			| (1 << CS00);   //PRESCALER =1024
}
void TIMER2_INIT(unsigned char compare_Value) {
	TCNT2 = 0;
	OCR2 = compare_Value;
	TCCR2 |= (1 << FOC2) | (1 << WGM21) | (1 << CS20) | (1 << CS21)
			| (1 << CS22);
}
void STOP_FAN() {
	/*
	 * @variable {on_Flag}:
	 * to ensure that PWM OC0 pin outputs 0 in case of stop state
	 */
	PORTC ^= (1 << PC6);
	PWM_TIMER_INIT(0);
	if (PORTC & 0x40)
		on_Flag = 1;
	else
		on_Flag = 0;

}
