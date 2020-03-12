/* 
 * Abstraction layer for the ATmega328p ADC 
 * Non-blocking read the ADC output as an alternative
 * to the blocking analogRead() method.
 *
 * This source file can be found under:
 * http://www.github.com/microfarad-de/Adc
 * 
 * Please visit:
 *   http://www.microfarad.de
 *   http://www.github.com/microfarad-de
 * 
 * Copyright (C) 2019 Karim Hraibi (khraibi at gmail.com)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include "Adc.h"
#include <assert.h>


AdcClass Adc;


void AdcClass::initialize (AdcPrescaler_t prescaler, AdcReference_t reference, uint8_t avgSamples, uint8_t numPins, AdcPin_t *adcPins) {
  this->reference = reference;
  assert (numPins <= ADC_NUM_PINS);
  this->numPins = numPins;
  this->avgSamples = avgSamples;
  for (uint8_t i = 0; i < numPins; i++) {
    this->adcPins[i] = adcPins[i];
  }
  ADCSRA =  _BV (ADEN);   // turn ADC on
  ADCSRA |= prescaler ;
}


void AdcClass::start (AdcPin_t adcPin) {
  uint8_t pin;

  if (working) return;

  pin = (uint8_t)adcPin;
  ADMUX  = reference | (pin & 0x07);      // select reference and input port
  bitSet (ADCSRA, ADSC);                  // start a conversion
  working = true;
}


int16_t AdcClass::readVal (void) {
  int16_t rv;

  // the ADC clears the bit when done
  if (bit_is_clear(ADCSRA, ADSC) && working) {
    rv = ADC; // read result
    working = false;
    return rv;
  }
  else {
    return -1;
  }
}


bool AdcClass::readAll (void) {
  int16_t adcVal;
  AdcPin_t pin = adcPins[pinIdx];
  // Start a new ADC measurement
  start (pin);

  // Check if ADC finished detecting the value
  adcVal = readVal ();

  // ADC finished
  if (adcVal >= 0) {
    if (avgCount == 0) result[pin] = 0;
    result[pin] += adcVal;
    if (avgCount == avgSamples - 1) result[pin] = result[pin] / avgSamples;
    pinIdx++;
    if (pinIdx >= numPins) {
      pinIdx = 0;
      avgCount++;
      if (avgCount >= avgSamples) {
        avgCount = 0;
        return true;
      }
    }
  }
  return false;
}

