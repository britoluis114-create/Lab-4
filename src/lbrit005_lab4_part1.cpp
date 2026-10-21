/*         Luis & lbrit005@ucr.edu:

            Discussion: 023

 *         Assignment: Lab #4  Exercise # 1

 *         Exercise Description: In this lab we will be using a 4D7S will be counting down

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: <https://youtu.be/ByMiRRPhyL0>

 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timerISR.h"
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
return (b ? (x | (0x01 << k)) : (x & ~(0x01 << k)) );
// Set bit to 1 Set bit to 0
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
enum states {start,idle, timer, buzzer} state; //TODO: finish the enum for the SM
void Tick() {
//TODO: declare your static variables here or declare it globally
// State Transistions
//TODO: complete transitions

static unsigned count = 9;
static unsigned char button = 0; //off for now
static unsigned char outNum[4] = {16, 16, 16, 16}; //display buffer
unsigned char i; //for loops
unsigned char pos; //for digit position
switch (state) {
    case start :
        state = idle;
        count = 9;
        button = 0;
        break;
    case idle:
        if (GetBit(PINC, 0) && button) {
            state = timer;
            button = 0;
            count = 9;
        }
        else if(!button && !GetBit(PINC, 0)) {
            button = 1;
        }
        break;
    case timer:
        if (TimerFlag) {
            TimerFlag = 0;
        count--;
        }
        if(count == 0) {
            state = buzzer;
        }

        break;
    case buzzer:
        if (TimerFlag) {
            TimerFlag = 0;
            state = idle;
            count = 9;
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
    case idle:
    outNum[0] = outNum[1] = outNum[2] = outNum[3] = 16; // all dashes
    PORTC = SetBit(PORTC, 3, 0);
        break;
    case timer:
    outNum[0] = count % 10; // the countdown
    outNum[1] = 17; // the 17 is blank
    outNum[2] = 17;
    outNum[3] = 17;
    PORTC = SetBit(PORTC, 3, 0);

        break;
    case buzzer:
    outNum[0] = 0;
    PORTC = SetBit(PORTC, 3, 1);
        break;
    default:
        outNum[0] = outNum[1] = outNum[2] = outNum[3] = 16;
        PORTC = SetBit(PORTC, 3, 0);
        break;

}
for(i =0; i < 4; i++) {
    pos = nums[outNum[i]];
    PORTB = (PORTB & 0xE1); //turn off all digits
    PORTD = (PORTD & 0x03) | ((pos & 0x3F) << 2);
    PORTB = (PORTB & 0xFE) | ((pos >> 6) & 0x01);
    PORTB = (PORTB & 0xE1) | ((~(0x01 << (i + 1))) & 0x1E);
    _delay_ms(5);
    

}
PORTB = (PORTB & 0xE1); //turn off all digits
}
int main(void)
{
//TODO: initialize all outputs and inputs
DDRB = 0x1F; //
PORTB = 0x00;
DDRC = 0x08; //buzzer on PC3
PORTC = 0x01; //pull-up on C0
DDRD = 0xFC; // a-g segments
PORTD = 0x00;
ADC_init();
TimerSet(1000); //TODO: Set your timer
TimerOn();

state = start; //TODO: initialize the state variable
while (1) {
    // keep refreshing display
    Tick();
    // only process timer tick once per second
}
return 0;
}
