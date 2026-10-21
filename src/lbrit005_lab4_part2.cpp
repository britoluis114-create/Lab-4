/*         Luis & lbrit005@ucr.edu:

            Discussion: 023

 *         Assignment: Lab #4  Exercise # 2

 *         Exercise Description: In this lab we will be using a 4D7S will be counting down where you left the potientometer

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: <https://youtu.be/nuh5gonrTik>

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

unsigned char outNum[4] = {16, 16, 16, 16}; // global so Display() can access it

void ADC_init() {
ADMUX = 0x40;
ADCSRA = 0x87;
ADCSRB = 0x00;
    //TODO: use your ADC_init from previous lab
}
unsigned int ADC_read(unsigned char chnl){
//TODO: use your ADC_read from previous lab
ADMUX |= 0xFF & chnl;
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

enum states {start, wait, calibrate, turn, timer} state; //TODO: finish the enum for the SM

void Tick() {
  //TODO: declare your static variables here or declare it globally
  // State Transistions
  //TODO: complete transitions 
    static unsigned short holdTime = 0;
    static unsigned short prevVal = 1;
    static unsigned short secTick = 0;
    static unsigned char longHeld = 0;
    static unsigned char currentVal = 0;
    static unsigned char startVal = 0;
    static unsigned char prevButton = 0;


    unsigned char potVal = (unsigned char)map(ADC_read(1), 0, 1023, 1, 15); // potentiometer on PC1
    unsigned char button = !(PINC & (1 << 0)); // button on PC0
    unsigned char buttonReleased = (!button && prevButton);
    prevButton = button;
  switch (state) {
    
        case start:
            state = wait;
            holdTime = 0;
            longHeld = 0;
            secTick = 0;
            currentVal = 0;
            startVal = 0;
            break;
        case wait:
            if(button){
              holdTime++;
              if(holdTime >= 2){

                longHeld = 1;
              }
            }
          if(buttonReleased){
              if(longHeld){
              state = calibrate;
              longHeld = 0;  
              }
              else if(holdTime > 0 && holdTime < 2){
                if(startVal > 0){state = turn;
                currentVal = startVal;
                secTick = 0;
                }
              
               holdTime = 0;
              }

          }  
        break;
        case calibrate:
            prevVal = potVal;
            if(button){
                holdTime++;
                if(holdTime >= 2){
                    longHeld = 1;
                }
              }
                if(buttonReleased){
                  if(longHeld){
                      startVal = prevVal;
                      currentVal = startVal;
                      state = wait;
                      longHeld = 0;
                  }
                  holdTime = 0;
              }
              break;
        case turn:
            secTick++;
            if (secTick >= 1) { // after 1 second, decrement the timer
                secTick = 0;
                if (currentVal > 0) currentVal--;
                if (currentVal == 0) 
                state = timer; // when timer reaches 0, enter timer state
            }
        break;
        case timer:
            if (button) { // if button is pressed, reset the system
              PORTC = SetBit(PORTC, 3, 0); // turn off buzzer  
              currentVal = startVal;
                secTick = 0;
                holdTime = 0;
                longHeld = 0;
                state = wait;
            }
        break;  
        default:
            state = start;
            break;
  }

  // State Actions
  //TODO: complete transitions
  switch (state) {

        case start:
        case wait:
            outNum[0] = 16;
            outNum[1] = 16;
            outNum[2] = 16;
            outNum[3] = 16;
            PORTC = SetBit(PORTC, 3, 0);
        break;  
        case calibrate:
            outNum[0] = 17;
            outNum[1] = 17;
            outNum[2] = 17;
            outNum[3] = potVal;
            PORTC = SetBit(PORTC, 3, 0);
        break;
        case turn:
            outNum[0] = currentVal;
            outNum[1] = 17;
            outNum[2] = 17;
            outNum[3] = 17;
            PORTC = SetBit(PORTC, 3, 0);
        break;
        case timer:
            outNum[0] = 16;
            outNum[1] = 16;
            outNum[2] = 16;
            outNum[3] = 16;
            PORTC = SetBit(PORTC, 3, 1);

        break;
        default:
        break;
}
}

void Display() {
    unsigned char i, pos;
    for (i = 0; i < 4; i++) {
        pos = nums[outNum[i]];
        PORTB = (PORTB & 0xE1);                                  // all digits OFF
        PORTD = (PORTD & 0x03) | ((pos & 0x3F) << 2);           // segments a-f
        PORTB = (PORTB & 0xFE) | ((pos >> 6) & 0x01);           // segment g
        PORTB = (PORTB & 0xE1) | ((~(0x01 << (i + 1))) & 0x1E); // digit ON
        _delay_ms(5);
    }
    PORTB = (PORTB & 0xE1); // all digits OFF

    if (state == timer) {
        PORTC ^= (1 << 3);  // toggle buzzer
    }

  }
int main(void)
{
  //TODO: initialize all outputs and inputs
    DDRB     = 0x1F;   // G id B0, and the display on B1-B4
    PORTB    = 0x00;
  
    DDRC    = 0x08; // buzzer on PC3, PC0 for button input, PC1 for the potentiometer input
    PORTC   = 0x01; // pull-up on C0

    DDRD   = 0xFC; // a-f segments on D2-D7
    PORTD  = 0x00;

    ADC_init();

  TimerSet(1000); //TODO: Set your timer
  TimerOn();
    while (1)
    {
      
	      Tick();      // Execute one synchSM tick
        while (!TimerFlag){
          Display(); // Update the display
        } // Wait for SM period  // Wait for SM period
        TimerFlag = 0;        // Lower flag
    }

    return 0;
}