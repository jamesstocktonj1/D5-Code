#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "ili934x.h"
#include <stdio.h>
#include <stdlib.h>

#define ATOMV 3.226


void adc_init(void);
uint16_t read_adc(uint8_t channel);

void watchdog_init(void);


//screen functions
void draw_bar(uint16_t value, uint8_t colour);


int main() {

    init_lcd();
    set_orientation(North);
    clear_screen();

    adc_init();

    int i;

    //setup refresh area rectangle
    rectangle refreshArea;
    refreshArea.top = 0;
    refreshArea.left = 0;
    refreshArea.bottom = LCDHEIGHT / 2;
    refreshArea.right = LCDWIDTH;
    

    while(1) {

        //clear screen
        //clear_screen();

        //only refresh changed area
        fill_rectangle(refreshArea, display.background);

        //set display cursor to top left
        display.x = 0;
        display.y = 0;

        for(i=0; i<8; i++) {

            char num[6];

            //output as raw 10-bit
            uint16_t temp = read_adc(i);
            itoa(temp, num, 10);

            //output as float voltage
            //float temp = read_adc(i) * ATOMV;
            //itoa(temp, num, 10);

            //write to display
            display_string("ADC: ");
            display_string(num);
            //display_string(" mV");

            display.x = 100;
            draw_bar(temp, GREEN);

            //move display cursor to start of next line
            display.y += 10;
            display.x = 0;
        }

        _delay_ms(200);
    }
    
    return 1;
}



void adc_init() {

    int i;

    //initialise pins A0-A8 as inputs
    for(i=0; i<8; i++) {
        DDRA &= ~_BV(i);
    }

    //setup mux register
    ADMUX = 0;                  //set mux channel to 0
    ADMUX |= _BV(REFS0);        //AVCC reference
    ADMUX |= _BV(ADLAR);        //left to right data

    //setup control register A
    ADCSRA |= _BV(ADPS2);       //set prescaler to 16
    //ADCSRA |= _BV(ADATE);       //auto trigger
	ADCSRA |= _BV(ADEN);		//enable adcs
    //ADCSRA |= _BV(ADIF);        //interrupt flag
	ADCSRA |= _BV(ADSC);	      //start conversion

    //setup control register B
	ADCSRB = 0;
}



uint16_t read_adc(uint8_t channel) {
    
    ADMUX = channel;            //set mux to channel number
    ADMUX |= _BV(REFS0);        //AVCC reference
    ADMUX |= _BV(ADLAR);        //left to right data

    ADCSRA |= _BV(ADSC);        //start conversion

    while(ADCSRA & _BV(ADSC));  //wait for end of conversion

    uint8_t lowerRegister = ADCL;       //read lower register
    uint8_t upperRegister = ADCH;       //read upper register

    return (upperRegister << 2) + (lowerRegister >> 6);         //convert lower/upper register to 10-bit number
}




void draw_bar(uint16_t value, uint8_t colour) {
    

    //store current position of cursor
    uint16_t curX, curY;
    curX = display.x;
    curY = display.y;

    //setup rectangle
    rectangle bar;
    bar.top = curY;
    bar.left = curX;
    bar.bottom = curY + 7;

    value = value / 4;
    bar.right = curX + value;

    fill_rectangle(bar, colour);
}