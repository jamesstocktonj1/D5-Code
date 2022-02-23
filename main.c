#include <avr/io.h>
#include <util/delay.h>
#include "lib/lcd.h"
#include "lib/ili934x.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "io.h"

#define ATOMV 3.226

#define ADC_REFRESH 4

#define BUFFER_SIZE 32

#define SCREEN_REFRESH 16




//screen functions
void draw_bar(uint16_t value, uint8_t colour);



void init_timers(void);

uint8_t updateDisplay = 1;




int main() {

    //initialise screen (and set orientation)
    init_lcd();
    set_orientation(North);
    clear_screen();

    //splash screen
    display_string("Hello World!");

    _delay_ms(2000);

    //init_timers();

    //sei();

    init_adc();
    init_pins();


    int i;

    //setup refresh area rectangle
    rectangle refreshArea;
    refreshArea.top = 0;
    refreshArea.left = 0;
    refreshArea.bottom = LCDHEIGHT / 2;
    refreshArea.right = LCDWIDTH;
    

    while(1) {


        if(1) {
            updateDisplay = 0;

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
        }

        _delay_ms(500);
    }
    
    return 1;
}




void init_timers() {
    

}



void draw_bar(uint16_t value, uint8_t colour) {

    //convert 0-1024 to 0-128
    value = value / 4;

    //setup rectangle
    rectangle bar;
    bar.top = display.y;
    bar.left = display.x;
    bar.bottom = display.y + 7;
    bar.right = display.x + value;
    
    fill_rectangle(bar, colour);
}