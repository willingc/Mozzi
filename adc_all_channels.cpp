/*
 * adc_all_channels.cpp
 *
 * Copyright 2012 Tim Barrass.
 *
 * This file is part of Mozzi.
 *
 * Mozzi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mozzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mozzi.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mozzi_analog.h"

#if !USE_AUDIO_INPUT

#include "Arduino.h"


//approach 3: adcEnableInterrupt(), adcReadAllChannels(), adcGetResult(), read all channels in background

/*
Programming note:
This is a separate file from mozzi_analog to avoid multiple definitions of the ISR(ADC_vect),
which is also in Mozziguts.cpp to handle audio input buffering.  It's an untidy solution.
Could the modules be better organised as classes?
*/

/*
This code is based on discussion between jRaskell, bobgardner, theusch, Koshchi, and code by jRaskell. 
http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=789581

Another approach discussed on the same page is to use free running mode on one channel only,
with (eg. 4) two resistor voltage dividers to define each input pseudo-channel.
The drawback there is lower resolution for each input because the 10-bit input 
range has to be divided between them.
*/


static volatile int sensors[NUM_ANALOG_INPUTS];
static volatile byte current_adc = 0;
static volatile boolean readComplete = false;

void adcReadAllChannels(){
	current_adc = 0;
	/*
	 //Set MUX channel
	 ADMUX = (1 << REFS0) | current_adc;
	 //Start A2D Conversions
	 ADCSRA |= (1 << ADSC);
	 */
	adcStartConversion(current_adc);
}


int adcGetResult(unsigned char channel_num){
	return sensors[channel_num];
}


/* This is called with when the adc finishes a conversion.
It puts the new reading into the sensors array and starts a new reading on the next channel.
*/
ISR(ADC_vect, ISR_BLOCK)
{
	static boolean secondRead = false;
	//Only record the second read on each channel
	if(secondRead){
		//sensors[current_adc] = ADCL | (ADCH << 8);
		//bobgardner: ..The compiler is clever enough to read the 10 bit value like this: val=ADC;
		sensors[current_adc] = ADC;
		current_adc++;
		if(current_adc > NUM_ANALOG_INPUTS){
			//Sequence complete.  Stop A2D conversions
			readComplete = true;
		}
		else{
			//Switch to next channel
			
			//ADMUX = (1 << REFS0) | current_adc;
			//ADCSRA |= (1 << ADSC);
			
			adcStartConversion(current_adc);
		}
		secondRead = false;
	}
	else{
		secondRead = true;
		ADCSRA |= (1 << ADSC);
	}
}

#endif
