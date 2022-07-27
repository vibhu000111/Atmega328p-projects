#define F_CPU 16000000L
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define SAMPLE_RATE 8000



#include "sounddata.h"

const int speakerPin = 11; // Can be either 3 or 11, two PWM outputs connected to Timer 2
volatile uint16_t sample;


void stopPlayback() {

	// Disable playback per-sample interrupt.
	TIMSK1 &= ~_BV(OCIE1A);

	// Disable the per-sample timer completely.
	TCCR1B &= ~_BV(CS10);

	// Disable the PWM timer.
	TCCR2B &= ~_BV(CS10);

	PORTB &= ~_BV(PORTB3);
	PORTB &= ~_BV(PORTB5);
}


// This is called at 8000 Hz to load the next sample.
ISR(TIMER1_COMPA_vect) {
	if(sample >= sounddata_length) {
		stopPlayback();
	} else {
		if(speakerPin==11) {
			OCR2A = pgm_read_byte(&sounddata_data[sample]);
		} else {
			OCR2B = pgm_read_byte(&sounddata_data[sample]);            
		}
		++sample;
	}
}

void startPlayback() {

	// Set up Timer 2 to do pulse width modulation on the speaker pin.

	// Set fast PWM mode 
	TCCR2A |= _BV(WGM21) | _BV(WGM20);
	TCCR2B &= ~_BV(WGM22);

	if(speakerPin == 11) {
		// Do non-inverting PWM on pin OC2A
		// On the Arduino this is pin 11.
		TCCR2A |= _BV(COM2A1);

		// Set initial pulse width to the first sample.
		OCR2A = pgm_read_byte(&sounddata_data[0]);
	} else {
		// Do non-inverting PWM on pin OC2B (p.155)
		// On the Arduino this is pin 3.
		TCCR2A = _BV(COM2B1);

		// Set initial pulse width to the first sample.
		OCR2B = pgm_read_byte(&sounddata_data[0]);
	}

	TCCR2B = _BV(CS10);	// no prescaler




	// Set up Timer 1 to send a sample every interrupt.


	// Set CTC mode (Clear Timer on Compare Match)
	TCCR1B  = _BV(WGM12);
	TCCR1B |= _BV(CS10);	// no prescaler

	// Set the compare register (OCR1A).
	OCR1A = F_CPU / SAMPLE_RATE;    // 16e6 / 8000 = 2000

	// Enable interrupt when TCNT1 == OCR1A (p.136)
	TIMSK1 = _BV(OCIE1A);

	sample = 0;
	sei();
}


int main(void) {
	DDRB = _BV(PORTB3) | _BV(PORTB5);
	PORTB = _BV(PORTB5);

	startPlayback();


	while(1) { }

	return 0;
}

