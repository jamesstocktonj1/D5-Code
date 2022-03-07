#include <avr/io.h>
#include <util/delay.h>
#include "lib/lcd.h"
#include "lib/ili934x.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "io.h"

#define ATOMV 3.226

#define MILLIS_VALUE 68

#define ADC_REFRESH 100
#define SCREEN_REFRESH 500

//graphics constants
#define HALF_WIDTH LCDWIDTH / 2
#define QUARTER_WIDTH LCDWIDTH / 4

#define HALF_HEIGHT LCDHEIGHT / 2
#define QUARTER_HEIGHT LCDHEIGHT / 4

#define COLUMN 100

//define colours
#define ORANGE 0xFD00

//define colour thresholds for tricolour bar
#define RED_THRESHOLD 870           //aprox 85%
#define ORANGE_THRESHOLD 717        //aprox 70%



//graphics functions
void draw_bar(uint16_t value, uint16_t colour);
void draw_tricolour_bar(uint16_t value);
void draw_signed_bar(int16_t value, uint16_t colour);
void draw_battery_state(battery state);
void draw_indicator(uint8_t state);

void draw_screen(void);

//initialise millisecond timer
void init_timer(void);
volatile uint32_t millis_timer;


//runs io functions
void read_inputs(void);
void write_outputs(void);


//main global values
//analog input values
uint16_t mains_capacity;
uint16_t busbar_voltage, busbar_current;
uint16_t wind_capacity, solar_capacity;

//digital input values
uint8_t load1_call, load2_call, load3_call;

//digital output values
uint8_t load1, load2, load3;
battery battery_state = DISCONNECTED;



ISR(TIMER0_OVF_vect) {
    millis_timer++;

    TCNT0 = MILLIS_VALUE;   //set count register value
}





int main() {

    //initialise screen (and set orientation)
    init_lcd();
    set_orientation(North);
    clear_screen();


    //initialise io (io.h)
    init_adc();
    init_pins();

    //init timer
    init_timer();
    sei();

    //splash screen
    display_string("Hello World!");

    _delay_ms(2000);

    

    while(1) {

        if((millis_timer % 1000) == 0) {

            //state = !state;
            //set_load1(state);

            switch(battery_state) {

                case CHARGING:
                    battery_state = DISCHARGING;
                    break;

                case DISCHARGING:
                    battery_state = DISCONNECTED;
                    break;

                default:
                    battery_state = CHARGING;
                    break;
            }
        }
        

        if((millis_timer % SCREEN_REFRESH) == 0) {
            
            draw_screen();
        }

        //state = !state;

        

        _delay_ms(1);
    }
    
    return 1;
}




void init_timer() {

    //set to normal mode, no output
    TCCR0A = 0x00;

    //clear register
    TCCR0B = 0x00;

    TCCR0B |= _BV(CS00) | _BV(CS01);        //set 64 clock divide

    TIMSK0 |= _BV(TOIE0);                    //enable overflow interrupt
    
    TCNT0 = MILLIS_VALUE;                   //set count register value

    //set timer variable to 0
    millis_timer = 0;
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

void draw_tricolour_bar(uint16_t value) {

    uint16_t colour = GREEN;

    if(value > RED_THRESHOLD) {
        colour = RED;
    }
    else if(value > ORANGE_THRESHOLD) {
        colour = ORANGE;
    }

    draw_bar(value, colour);
}

void draw_signed_bar(int16_t value, uint16_t colour) {

    //converty +-512 to +-64
    value = value / 4;

    //setup rectangle
    rectangle bar;
    bar.top = display.y;
    bar.bottom = display.y + 7;

    //sets the l/r coords depending on whether the value is positive or negative
    bar.left    = (value < 0) ? display.x + 64 + value : display.x + 64;
    bar.right   = (value < 0) ? display.x + 64 : display.x + 64 + value;

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


void draw_screen() {

    //setup refresh area rectangle
    rectangle refreshArea;
    refreshArea.top = 0;
    refreshArea.left = 0;
    refreshArea.bottom = LCDHEIGHT / 2;
    refreshArea.right = LCDWIDTH;

    //clear screen
    //clear_screen();

    //only refresh changed area
    fill_rectangle(refreshArea, display.background);

    //set display cursor to top left
    display.x = 0;
    display.y = 0;

    int i;

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
        //draw_indicator(state);

        display.y += 10;
        display.x = 0;
    }

    display_string("Battery State:");
    display.x = HALF_WIDTH;

    draw_battery_state(battery_state);



    display.y += 20;
    display.x = 0;

    char num[8];
    ltoa(millis_timer, num, 10);

    display_string("Timer Value: ");
    display_string(num);
}



void read_inputs(void) {

    //read analog values
    mains_capacity = get_mains_capacity();

    busbar_voltage = get_busbar_voltage();
    busbar_current = get_busbar_current();

    wind_capacity = get_wind_capacity();
    solar_capacity = get_solar_capacity();


    //read digital values
    load1_call = get_load1();
    load2_call = get_load2();
    load3_call = get_load3();
}



void write_outputs(void) {

    //set output values
    set_load1(load1);
    set_load2(load2);
    set_load3(load3);

    set_battery_state(battery_state);
}