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
#define SCREEN_REFRESH 16

//graphics constants
#define HALF_WIDTH LCDWIDTH / 2
#define QUARTER_WIDTH LCDWIDTH / 4




//graphics functions
void draw_bar(uint16_t value, uint16_t colour);
void draw_battery_state(battery state);
void draw_indicator(uint8_t state);

//initialise millisecond timer
void init_timers(void);
volatile uint32_t millis_timer;



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
    
    uint8_t state = 0;

    battery bat_state = CHARGING;

    while(1) {

        set_load1(state);

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

                display.x = HALF_WIDTH;
                draw_bar(temp, GREEN);

                //move display cursor to start of next line
                display.y += 10;
                display.x = 0;
            }

            for(i=0; i<4; i++) {

                char num[1];
                itoa(i, num, 10);

                display_string("Indicator ");
                display_string(num);
                display_string(": ");

                display.x = HALF_WIDTH;
                draw_indicator(state);

                display.y += 10;
                display.x = 0;
            }

            display_string("Battery State:");
            display.x = HALF_WIDTH;

            draw_battery_state(bat_state);
        }

        state = !state;

        switch(bat_state) {

            case CHARGING:
                bat_state = DISCHARGING;
                break;

            case DISCHARGING:
                bat_state = DISCONNECTED;
                break;

            default:
                bat_state = CHARGING;
                break;
        }

        _delay_ms(500);
    }
    
    return 1;
}




void init_timers() {
    

}



void draw_bar(uint16_t value, uint16_t colour) {

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

void draw_battery_state(battery state) {

    //create indicator square of size 7x7 (pixel height square)
    rectangle ind;
    ind.top = display.y;
    ind.left = display.x;
    ind.bottom = display.y + 7;
    ind.right = display.x + 7;

    //color variable
    uint16_t col;

    switch(state) {

        case CHARGING:
            col = GREEN;
            break;

        case DISCHARGING:
            col = RED;
            break;

        default:
            col = YELLOW;
            break;
    }

    //draw indicator
    fill_rectangle(ind, col);
}

void draw_indicator(uint8_t state) {

    //create indicator square of size 7x7 (pixel height square)
    rectangle ind;
    ind.top = display.y;
    ind.left = display.x;
    ind.bottom = display.y + 7;
    ind.right = display.x + 7;

    //color variable (green or read)
    uint16_t col = (state) ? GREEN : RED;

    //draw indicator
    fill_rectangle(ind, col);
}
