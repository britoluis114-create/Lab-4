/*         Luis & lbrit005@ucr.edu:

            Discussion: 023

 *         Assignment: Lab #4  Exercise # 3

 *         *         Exercise Description: In this lab we will be using a 4D7S will be counting down when the photoresistor is bright and resets when it reaches zero. 


 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: <https://youtu.be/5_GUL1NYmNA>

 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "timerISR.h"

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

//TODO: Use nums[] and outNum[] from the previous lab. The wiring is slightly different, modify it so it works as intended
int nums[18] = {
    0b0111111, // 0
    0b0000110, // 1
    0b1011011, // 2
    0b1001111, // 3
    0b1100110, // 4
    0b1101101, // 5
    0b1111101, // 6
    0b0000111, // 7
    0b1111111, // 8
    0b1101111, // 9
    0b1110111, // a
    0b1111100, // b
    0b0111001, // c
    0b1011110, // d
    0b1111001, // E
    0b1110001,  // f
    0b1000000,  // -
    0b0000000 // blank
};

unsigned char outNum[4] = {16, 16, 16, 16}; // global

void ADC_init() {
    //TODO: use your ADC_init from previous lab
    ADMUX = 0x40;
    ADCSRA = 0x87;
    ADCSRB = 0x00;
}



unsigned int ADC_read(unsigned char chnl){
  //TODO: use your ADC_read from previous lab
  ADMUX = (ADMUX & 0xF0) | (chnl & 0x0F);
  ADCSRA = ADCSRA | 0x40;
  while((ADCSRA >> 6)&0x01) {}
  uint8_t low, high;
  low = ADCL;
  high = ADCH;
  return ((high << 8) | low);
}

// Provided map()
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

enum states {start, idle, bright, dim, buzzer} state; //TODO: finish the enum for the SM

void Tick() {
  //TODO: declare your static variables here or declare it globally
    static unsigned char button = 0;
    static unsigned char photoVal;
    photoVal = ADC_read(2); // read photoresistor on channel 3
    static unsigned char countdown =9;
    static unsigned char prevBut = 0;
    static unsigned int tick = 0;
    static unsigned int buzz = 0;
    unsigned char i;
    unsigned char pos;

    static unsigned press = 0;
    prevBut = button;
    button = !(PINC & (1 << 0));
    press = (button && !prevBut);

  // State Transistions
  //TODO: complete transitions 
  switch (state) {
      case start:
          state = idle;
      break;
      case idle:
          if(press){
            countdown = 9;
            tick = 0;
              state = bright;
          }
      break;
      case bright:
          if(countdown == 0){
            state = buzzer;
          }
          else if(photoVal <= 50){
            state = dim; }
      break;
      case dim:
          if(photoVal > 50){
          state = bright;
          }
      break;
      case buzzer:
          if(buzz >= 50){
           buzz = 0;
           countdown = 9; 
          state = idle;
          }
      break;
      default:
          state = idle;
          break;

  }

  // State Actions
  //TODO: complete transitions
  switch (state) {
    case start:
    break;
    case idle:
        outNum[0] = outNum[1] = outNum[2] = outNum[3] = 16;
    PORTC = SetBit(PORTC, 3, 0);
    break;
    case bright:
      if(tick >= 50){

        countdown--;
        tick = 0;
      }    
      tick++;
    outNum[0] = countdown % 10;
    outNum[1] = 17;
    outNum[2] = 17;
    outNum[3] = 17;
    PORTC = SetBit(PORTC, 3, 0);
    break;
    case dim:
    outNum[0] = countdown % 10;
    outNum[1] = 17;
    outNum[2] = 17;
    outNum[3] = 17;
    PORTC = SetBit(PORTC, 3, 0);
    break;
    case buzzer:
    buzz++;
    outNum[0] = 17;
    outNum[1] = 17;
    outNum[2] = 17;
    outNum[3] = 17;
    PORTC = SetBit(PORTC, 3, 1);
        break;
    default:
        outNum[0] = outNum[1] = outNum[2] = outNum[3] = 16;
        PORTC = SetBit(PORTC, 3, 0);
        break;
}
  for(i = 0; i < 4; i++) {
        pos = nums[outNum[i]];
        PORTB = (PORTB & 0xE1);
        PORTD = (PORTD & 0x03) | ((pos & 0x3F) << 2);
        PORTB = (PORTB & 0xFE) | ((pos >> 6) & 0x01);
        PORTB = (PORTB & 0xE1) | ((~(0x01 << (i + 1))) & 0x1E);
        _delay_ms(5);
    }
    PORTB = (PORTB & 0xE1);
}
int main(void)
{
  //TODO: initialize all outputs and inputs
    DDRB     = 0x1F;
    PORTB    = 0x00;
  
    DDRC    = 0x08;
    PORTC   = 0x01;

    DDRD   = 0xFC;
    PORTD  = 0x00;

    ADC_init();

  TimerSet(20); //TODO: Set your timer
  TimerOn();
    while (1)
    {
      
	      Tick();      // Execute one synchSM tick
        while (!TimerFlag){}  // Wait for SM period
        TimerFlag = 0;        // Lower flag
    }

    return 0;
}