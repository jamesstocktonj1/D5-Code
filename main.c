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

// graphics constants
#define HALF_WIDTH LCDWIDTH / 2
#define QUARTER_WIDTH LCDWIDTH / 4

#define HALF_HEIGHT LCDHEIGHT / 2
#define QUARTER_HEIGHT LCDHEIGHT / 4

#define LINDENT 10

#define COLUMN 100

// define colours
#define ORANGE 0xFD00

// define colour thresholds for tricolour bar
#define RED_THRESHOLD 870    // aprox 85%
#define ORANGE_THRESHOLD 717 // aprox 70%

// TODO
// implement graphics for power
// implement load names

// graphics functions
void draw_bar(uint16_t value, uint16_t colour);
void draw_tricolour_bar(uint16_t value);
void draw_signed_bar(int16_t value, uint16_t colour);
void draw_battery_state(battery state);
void draw_indicator(uint8_t state);
void draw_status(char *status, uint16_t colour);

void splash_screen(void);
void draw_screen(void);

// initialise millisecond timer
void init_timer(void);
volatile uint32_t millis_timer;

// runs io functions
void read_inputs(void);
void write_outputs(void);

// main algorithm
void algorithm(void);
void load_switch(battery bat, uint8_t l1, uint8_t l2, uint8_t l3);

// main global values
// analog input values
uint16_t busbar_voltage, busbar_current;
float wind_capacity, solar_capacity;

// digital input values
uint8_t load1_call, load2_call, load3_call;

// Analogue output values
uint16_t mains_capacity;

// digital output values
uint8_t load1, load2, load3;
battery battery_state = DISCONNECTED;
int8_t battery_charge;

ISR(TIMER0_OVF_vect)
{
    millis_timer++;

    TCNT0 = MILLIS_VALUE; // set count register value
}

int main()
{

    // initialise screen (and set orientation)
    init_lcd();
    set_orientation(North);
    clear_screen();

    // initialise io (io.h)
    init_adc();
    init_pins();

    // init timer
    init_timer();
    sei();

    // splash screen
    splash_screen();

    _delay_ms(2000);

    while (1)
    {

        /*
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
        */

        if ((millis_timer % SCREEN_REFRESH) == 0)
        {

            read_inputs();

            draw_screen();
            //write algorithm results
        }

        // state = !state;

        _delay_ms(1);
    }

    return 1;
}

// Main algorithm to be implemented in project
// Average score of 79.16%
void algorithm(void)
{
    uint32_t current_time = millis_timer;
    uint8_t offpeak = ((current_timer <= (7 * 60 * 1000) && battery_charge <= 2) || (current_timer >= (22 * 60 * 1000) && battery_charge < 0));

    // Set constants to zero
    //int housesLost = 0;
    float MainsReq = 0;

    // Calculate power necessary
    float PowerRequired = 1.2 * load1_call + 2 * load2_call + 0.8 * load3_call;
    float PowerDeficit = PowerRequired - wind_capacity - solar_capacity;

    //float mainsSupply = (V2_2 - V2_5) - a.Wind[i] - a.PV[i];

    // Charge battery if excess renewable
    if (PowerDeficit < -1)
    {
        battery_state = CHARGING;
        MainsReq = 0;
    }

    // Charge battery if some spare renewable and make up the difference with mains
    // Change to -0.1 increases score but is probably specific to these tests
    else if (-1 <= PowerDeficit && PowerDeficit < 0)
    {
        battery_state = CHARGING;
        MainsReq = 5 * (PowerDeficit + 1);
    }

    // Offpeak

    // Charge battery if small load and offpeak
    // Charge to +2 before peak
    // Charge to 0 after peak
    else if ((0 <= PowerDeficit && PowerDeficit < 1) && offpeak)
    {
        load_switch(CHARGING, load1_call, load2_call, load3_call);
        MainsReq = 5 * (PowerDeficit + 1);
    }

    // Charge battery, switch off lights if medium load and offpeak
    else if ((1 <= PowerDeficit && PowerDeficit < 1.8) && offpeak)
    {
        load_switch(CHARGING, load1_call, load2_call, 0);
        MainsReq = 5 * (PowerDeficit + 0.2);
    }

    // Charge battery, switch off lifters if high load and offpeak
    else if ((1.8 <= PowerDeficit && PowerDeficit < 3) && offpeak)
    {
        load_switch(CHARGING, load1_call, 0, load3_call);
        MainsReq = 5 * (PowerDeficit - 1);
    }

    // Battery at -2

    // Small load, supply with mains
    else if ((0 <= PowerDeficit && PowerDeficit < 2) && battery_charge <= -2)
    {
        load_switch(DISCONNECTED, load1_call, load2_call, load3_call);
        MainsReq = 5 * (PowerDeficit);
    }

    // Medium load, switch off lights supply with mains
    else if ((2 <= PowerDeficit && PowerDeficit < 2.8) && battery_charge <= -2)
    {
        load_switch(DISCONNECTED, load1_call, load2_call, 0);
        MainsReq = 5 * (PowerDeficit - 0.8);
    }

    // High load, switch off lifters supply with mains
    else if ((2.8 <= PowerDeficit && PowerDeficit <= 4) && battery_charge <= -2)
    {
        load_switch(DISCONNECTED, load1_call, 0, load3_call);
        MainsReq = 5 * (PowerDeficit - 2);
    }

    // Battery above -2

    // Small load, discharge battery and supply with mains
    else if ((0 <= PowerDeficit && PowerDeficit < 3) && battery_charge > -2)
    {
        load_switch(DISCHARGING, load1_call, load2_call, load3_call);
        MainsReq = 5 * (PowerDeficit - 1);
    }

    // Medium load, switch off lights discharge battery and supply with mains
    else if ((3 <= PowerDeficit && PowerDeficit < 3.8) && battery_charge > -2)
    {
        load_switch(DISCHARGING, load1_call, load2_call, 0);
        MainsReq = 5 * (PowerDeficit - 1.8);
    }

    // High load, switch off lifters dischage battery and supply with mains
    else if ((3.8 <= PowerDeficit && PowerDeficit <= 4) && battery_charge > -2)
    {
        load_switch(DISCHARGING, load1_call, 0, load3_call);
        MainsReq = 5 * (PowerDeficit - 3);
    }

    // If unsure, switch off lifters dont make battery better or worse, supply with mains
    else
    {
        load_switch(DISCONNECTED, load1_call, 0, load3_call);
        MainsReq = 10;
    }
}

// Set loads to appropriate values
void load_switch(battery bat, uint8_t l1, uint8_t l2, uint8_t l3)
{
    battery_state = bat;
    load1 = l1;
    load2 = l2;
    load3 = l3;
}

void init_timer()
{
    // set to normal mode, no output
    TCCR0A = 0x00;

    // clear register
    TCCR0B = 0x00;

    TCCR0B |= _BV(CS00) | _BV(CS01); // set 64 clock divide

    TIMSK0 |= _BV(TOIE0); // enable overflow interrupt

    TCNT0 = MILLIS_VALUE; // set count register value

    // set timer variable to 0
    millis_timer = 0;
}

void draw_bar(uint16_t value, uint16_t colour)
{

    // convert 0-1024 to 0-128
    value = value / 8;

    // setup rectangle
    rectangle bar;
    bar.top = display.y;
    bar.left = display.x;
    bar.bottom = display.y + 7;
    bar.right = display.x + value;

    fill_rectangle(bar, colour);
}

void draw_tricolour_bar(uint16_t value)
{

    uint16_t colour = GREEN;

    if (value > RED_THRESHOLD)
    {
        colour = RED;
    }
    else if (value > ORANGE_THRESHOLD)
    {
        colour = ORANGE;
    }

    draw_bar(value, colour);
}

void draw_signed_bar(int16_t value, uint16_t colour)
{

    // converty +-512 to +-64
    value = value / 4;

    // setup rectangle
    rectangle bar;
    bar.top = display.y;
    bar.bottom = display.y + 7;

    // sets the l/r coords depending on whether the value is positive or negative
    bar.left = (value < 0) ? display.x + 64 + value : display.x + 64;
    bar.right = (value < 0) ? display.x + 64 : display.x + 64 + value;

    fill_rectangle(bar, colour);
}

void draw_battery_state(battery state)
{

    // create indicator square of size 7x7 (pixel height square)
    rectangle ind;
    ind.top = display.y;
    ind.left = display.x;
    ind.bottom = display.y + 7;
    ind.right = display.x + 7;

    // color variable
    uint16_t col;

    switch (state)
    {

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

    // draw indicator
    fill_rectangle(ind, col);
}

void draw_indicator(uint8_t state)
{

    // create indicator square of size 7x7 (pixel height square)
    rectangle ind;
    ind.top = display.y;
    ind.left = display.x;
    ind.bottom = display.y + 7;
    ind.right = display.x + 7;

    // color variable (green or read)
    uint16_t col = (state) ? GREEN : RED;

    // draw indicator
    fill_rectangle(ind, col);
}

void draw_status(char *status, uint16_t colour)
{

    rectangle stat;
    stat.top = 270;
    stat.bottom = 310;
    stat.left = 60;
    stat.right = 180;

    fill_rectangle(stat, colour);

    display.x = 80;
    display.y = 286;
    display_string(status);
}

void draw_screen()
{

    // clear changing area of the screen
    clear_screen();

    // display wind capacity
    display.y = 20;

    display.x = LINDENT;
    display_string("Wind:");

    display.x = COLUMN;
    draw_bar(wind_capacity, GREEN);

    // display solar capacity
    display.y = 40;

    display.x = LINDENT;
    display_string("Solar:");

    display.x = COLUMN;
    draw_bar(solar_capacity, GREEN);

    // display mains capacity
    display.y = 60;

    display.x = LINDENT;
    display_string("Mains:");

    display.x = COLUMN;
    draw_bar(mains_capacity, GREEN);

    // display battery status
    display.y = 80;

    display.x = LINDENT;
    display_string("Battery:");

    display.x = COLUMN;
    draw_battery_state(battery_state);

    display.x = COLUMN + 15;
    draw_bar(battery_charge, GREEN);

    // display call 1
    display.y = 120;

    display.x = LINDENT;
    display_string("Call 1:");

    display.x = COLUMN;
    draw_indicator(load1_call);

    display.x = COLUMN * 1.5;
    draw_indicator(load1);

    // display call 2
    display.y = 140;

    display.x = LINDENT;
    display_string("Call 2:");

    display.x = COLUMN;
    draw_indicator(load2_call);

    display.x = COLUMN * 1.5;
    draw_indicator(load2);

    // display call 3
    display.y = 160;

    display.x = LINDENT;
    display_string("Call 3:");

    display.x = COLUMN;
    draw_indicator(load3_call);

    display.x = COLUMN * 1.5;
    draw_indicator(load3);

    // display busbar voltage
    display.y = 200;

    display.x = LINDENT;
    display_string("Busbar Voltage:");

    display.x = COLUMN;
    draw_bar(busbar_voltage, GREEN);

    // display busbar current
    display.y = 220;

    display.x = LINDENT;
    display_string("Busbar Current:");

    display.x = COLUMN;
    draw_bar(busbar_current, GREEN);

    // display busbar power
    display.y = 240;

    display.x = LINDENT;
    display_string("Busbar Power:");

    uint16_t busbar_power = 0;
    busbar_power = (busbar_current * busbar_voltage) / 1024;

    display.x = COLUMN;
    draw_bar(busbar_power, GREEN);

    // draw status box
    draw_status("Good", GREEN);
}

/*
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
    //fill_rectangle(refreshArea, display.background);

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
}*/

void read_inputs(void)
{

    // read analog values
    mains_capacity = get_mains_capacity();

    busbar_voltage = get_busbar_voltage();
    busbar_current = get_busbar_current();

    wind_capacity = get_wind_capacity();
    solar_capacity = get_solar_capacity();

    // read digital values
    load1_call = get_load1();
    load2_call = get_load2();
    load3_call = get_load3();
}

void write_outputs()
{

    // set output values
    set_load1(load1);
    set_load2(load2);
    set_load3(load3);

    set_battery_state(battery_state);
}

void splash_screen()
{
    rectangle a;
	a.left = 45;
	a.right = 150;
	a.top  = 270;
	a.bottom = 290;
	fill_rectangle(a, WHITE);
	a.left = 45;
	a.right = 65;
	a.top = 110;
	a.bottom = 270;
	fill_rectangle(a, WHITE);
	a.left = 45;
	a.right = 150;
	a.top = 90;
	a.bottom = 110;
	fill_rectangle(a, WHITE);
	a.left = 170;
	a.right = 190;
	a.top = 130;
	a.bottom = 250;
	fill_rectangle(a, WHITE);
	a.left = 150;
	a.right = 170;
	a.top = 110;
	a.bottom = 130;
	fill_rectangle(a, WHITE);
	a.left = 150;
	a.right = 170;
	a.top = 250;
	a.bottom = 270;
	fill_rectangle(a, WHITE);
}