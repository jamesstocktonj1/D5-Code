#include <avr/io.h>
#include <util/delay.h>
#include "lib/lcd.h"
#include "lib/ili934x.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


//algorithm constants
#define AMP 256
#define HALF_AMP 128

//scaled ranges to amps(256)
#define BAT_CHARGE 256
#define BAT_DISCHARGE 256

#define LOAD1_MAX 307
#define LOAD2_MAX 512
#define LOAD3_MAX 205

#define WIND_MAX 768
#define SOLAR_MAX 256
#define MAINS_MAX 512

#define BUSI_MAX 1024
#define BUSV_MAX 10

#define CHARGE_TO 120000
#define FINAL_CHARGE 20000
#define DISCHARGE_TO -1200000

//port adjustments
#define WIND_CONST 1.4
#define SOLAR_CONST 1.42

#define MAINS_CONST 0.25
#define MAINS_FEEDBACK 1.0

#define BUSI_CONST 1
#define BUSV_CONST 9.77

#define LOOP_SPEED 200

// TODO
// implement load names

// graphics functions
void draw_bar(uint16_t value, uint16_t colour);
void draw_tricolour_bar(uint16_t value);
void draw_signed_bar(int16_t value, uint16_t colour);
void draw_battery_state(battery state);
void draw_indicator(uint8_t state);
void draw_status(char* status, uint16_t colour);

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
uint16_t wind_capacity, solar_capacity;

// digital input values
uint8_t load1_call, load2_call, load3_call;

// Analogue output values
uint16_t mains_request;
uint16_t total_load;

// digital output values
uint8_t load1, load2, load3;
battery battery_state = DISCONNECTED;
int32_t battery_charge;

uint8_t initial_charge = 0;
uint32_t prev_time;


char batteryMessage[12];
char mainsMessage[12];
uint16_t statusColour;

ISR(TIMER0_OVF_vect) {
    millis_timer++;

    TCNT0 = MILLIS_VALUE; // set count register value
}

int main() {

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

    while (1) {

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

        if ((millis_timer % SCREEN_REFRESH) == 0) {

            //read_inputs();
            //draw_screen();
            // write algorithm results
        }

        // state = !
        //state;
        draw_screen();
        algorithm();
        _delay_ms(250);
    }

    return 1;
}



// Main algorithm to be implemented in project
// Average score of 79.16%
void algorithm(void) {

    //store current time
    uint32_t current_time = millis_timer;
    uint32_t delta_time = current_time - prev_time;
    uint32_t current_second = current_time / 1000;

    prev_time = current_time;

    if (battery_state == CHARGING) {
        battery_charge += delta_time;
    }
    if (battery_state == DISCHARGING) {
        battery_charge -= delta_time;
    }

    busbar_current = get_busbar_current();

    //calculate difference between actual busbar current and what was requested (previous values)
    uint16_t mainsDeficit = (mains_request == 0 || (busbar_current * BUSI_CONST) > mains_request) ? 0 : (busbar_current * BUSI_CONST) - (mains_request);// + (wind_capacity * WIND_CONST) + (solar_capacity * SOLAR_CONST));
    //uint16_t mainsDeficit = 0;

    //decide if off peak time (batter should charge)
    //uint8_t offpeak = ((current_time <= (8 * 60 * 1000) && battery_charge <= 2) || (current_time >= (22 * 60 * 1000) && battery_charge < 0));

    //read in new values
    read_inputs();

    //calculate required load and sum of renewable sources
    //uint16_t loadSum = load1_call + load2_call + load3_call + mainsDeficit;
    uint16_t loadSum = 0;
    loadSum += (load1_call) ? LOAD1_MAX : 0;
    loadSum += (load2_call) ? LOAD2_MAX : 0;
    loadSum += (load3_call) ? LOAD3_MAX : 0;

    loadSum += mainsDeficit * MAINS_FEEDBACK;

    //take a copy of the pre-feedback load
    total_load = loadSum + 0;

    //adding the difference in busbar current to the loads so that the difference in current is made up (or loads taken out)
    loadSum += mainsDeficit * MAINS_FEEDBACK;

    //set default values for loads
    load1 = load1_call;
    load2 = load2_call;
    load3 = load3_call;

    //set default values for mains and battery
    //battery_state = DISCONNECTED;
    mains_request = 0;

    uint16_t renewableSum = (wind_capacity * WIND_CONST) + (solar_capacity * SOLAR_CONST);

    //calculate power required (excess power if negative)
    int16_t powerDeficit = loadSum - renewableSum;
    uint16_t powerExcess = (powerDeficit < 0) ? (powerDeficit * -1) : 0;

    


    //make decisions on battery status

    //charging if:
    //time < 7 and charge < 2
    //time > 22 and charge < 0
    //spare renewables
    if ((current_second < (8 * 60) && battery_charge < CHARGE_TO) || (current_second > (22 * 60) && battery_charge < FINAL_CHARGE) || (powerExcess > 0)) {
        battery_state = CHARGING;
        powerDeficit += AMP;

        //batteryMessage = "Charging";
        strcpy(batteryMessage, "Charging");
        statusColour = GREEN;

        //initial_charge = (battery_charge > CHARGE_TO) ? 0 : 1;
    }

    //discharging if:
    //deficit < 1 and charge > -2
    //t > 22 and charge > 1
    else if ((powerDeficit > AMP && battery_charge > DISCHARGE_TO && current_second > (8 * 60)) || (current_second > (22 * 60) && battery_charge > 15000)) {
        battery_state = DISCHARGING;
        powerDeficit -= AMP;

        //batteryMessage = 'Discharging';
        strcpy(batteryMessage, "Discharging");
        statusColour = (battery_charge < 0) ? ORANGE : GREEN;
    }

    //else disconnect
    else {
        battery_state = DISCONNECTED;
    }

    //make decisions on mains status

    if (powerDeficit < 0) {
        mains_request = 0;
        strcpy(mainsMessage, "");
    }
    //if mains can supply remaining power
    else if (powerDeficit <= MAINS_MAX) {
        mains_request = powerDeficit;

        //mainsMessage = (batteryMessage == "Charging" || batteryMessage == "Discharging") ? " with Mains" : "Mains";
        if(battery_state == DISCONNECTED) {
            strcpy(mainsMessage, "Mains");
        }
        else {
            strcpy(mainsMessage, " with Mains");
        }
        
    }

    //if cant supply remaining power then reduce loads
    else {
        //reduceLoad(powerDeficit);

        uint16_t makeUpPower = powerDeficit - MAINS_MAX;

        //switch off smallest load first (load 3)
        if (makeUpPower <= LOAD3_MAX) {
            load1 = load1_call;
            load2 = load2_call;
            load3 = 0;

            powerDeficit -= LOAD3_MAX;
        }

        //switch off load 2 to lose the least amount of marks
        else if (LOAD3_MAX < makeUpPower && makeUpPower <= LOAD2_MAX) {
            load1 = load1_call;
            load2 = 0;
            load3 = load1_call;

            powerDeficit -= LOAD1_MAX;
        }

        //switch off load 3 and 2 if cant keep up
        else if (LOAD2_MAX < makeUpPower && makeUpPower <= (LOAD2_MAX + LOAD3_MAX)) {
            load1 = load1_call;
            load2 = 0;
            load3 = 0;

            powerDeficit -= (LOAD1_MAX + LOAD3_MAX);
        }

        //worst case scenario
        else {
            load1 = 0;
            load2 = 0;
            load3 = 0;

            powerDeficit -= (LOAD1_MAX + LOAD2_MAX + LOAD3_MAX);
        }

        mains_request = powerDeficit;

        statusColour = RED;
        strcpy(mainsMessage, "Mains Overload!");
    }

    write_outputs();
}



void init_timer() {
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

void draw_bar(uint16_t value, uint16_t colour) {

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

void draw_tricolour_bar(uint16_t value) {

    uint16_t colour = GREEN;

    if (value > RED_THRESHOLD) {
        colour = RED;
    }
    else if (value > ORANGE_THRESHOLD) {
        colour = ORANGE;
    }

    draw_bar(value, colour);
}

void draw_signed_bar(int16_t value, uint16_t colour) {

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

void draw_battery_state(battery state) {

    // create indicator square of size 7x7 (pixel height square)
    rectangle ind;
    ind.top = display.y;
    ind.left = display.x;
    ind.bottom = display.y + 7;
    ind.right = display.x + 7;

    // color variable
    uint16_t col;

    switch (state) {

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

void draw_indicator(uint8_t state) {

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

void draw_status(char* status, uint16_t colour) {

    display.x = 80;
    display.y = 286;
    display_string(status);

    display.x += 10;

    //status colour indicator
    rectangle ind;
    ind.top = display.y;
    ind.left = display.x;
    ind.bottom = display.y + 7;
    ind.right = display.x + 7;

    fill_rectangle(ind, colour);
}

void draw_screen() {

    // clear changing area of the screen
    clear_screen();

    char temp[8];

    // display wind capacity
    display.y = 20;

    display.x = LINDENT;
    display_string("Wind:");

    ltoa(wind_capacity, temp, 10);
    display_string(temp);

    display.x = COLUMN;
    draw_bar(wind_capacity, GREEN);

    // display solar capacity
    display.y = 40;

    display.x = LINDENT;
    display_string("Solar:");

    ltoa(solar_capacity, temp, 10);
    display_string(temp);

    display.x = COLUMN;
    draw_bar(solar_capacity, GREEN);

    // display mains capacity
    display.y = 60;

    display.x = LINDENT;
    display_string("Mains:");

    ltoa(mains_request, temp, 10);
    display_string(temp);

    display.x = COLUMN;
    draw_bar(mains_request, GREEN);

    // display battery status
    display.y = 80;

    display.x = LINDENT;
    display_string("Battery:");

    display.x = COLUMN;
    draw_battery_state(battery_state);


    display.x = COLUMN + 15;
    //draw_bar(battery_charge, GREEN);
    ltoa(battery_charge, temp, 10);
    display_string(temp);

    // display call 1
    display.y = 120;

    display.x = LINDENT;
    display_string("Pumps:");

    display.x = COLUMN;
    draw_indicator(load1_call);

    display.x = COLUMN * 1.5;
    draw_indicator(load1);

    // display call 2
    display.y = 140;

    display.x = LINDENT;
    display_string("Lifts:");

    display.x = COLUMN;
    draw_indicator(load2_call);

    display.x = COLUMN * 1.5;
    draw_indicator(load2);

    // display call 3
    display.y = 160;

    display.x = LINDENT;
    display_string("Lights:");

    display.x = COLUMN;
    draw_indicator(load3_call);

    display.x = COLUMN * 1.5;
    draw_indicator(load3);

    //display total load
    display.y = 180;

    display.x = LINDENT;
    display_string("Total Load:");

    display.x = COLUMN;
    draw_bar(total_load, GREEN);

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
    busbar_power = ((busbar_current >> 5) * (busbar_voltage >> 5));

    display.x = COLUMN;
    draw_bar(busbar_power, GREEN);
    
    //display current time
    display.y = 260;
    uint16_t curTime = millis_timer / 1000;
    ltoa(curTime, temp, 10);

    display.x = LINDENT;
    display_string("Time: ");

    display.x = COLUMN;
    display_string(temp);
    display_string(" s");

    // draw status box
    display.y = 300;

    display.x = LINDENT;

    rectangle ind;
    ind.top = display.y;
    ind.left = display.x;
    ind.bottom = display.y + 7;
    ind.right = display.x + 7;

    fill_rectangle(ind, statusColour);

    display.x = COLUMN;
    display_string(batteryMessage);
    display_string(mainsMessage);
}


void read_inputs(void) {

    // read analog values
    //mains_capacity = get_mains_capacity();

    busbar_voltage = get_busbar_voltage();
    busbar_current = get_busbar_current();

    wind_capacity = get_wind_capacity();
    solar_capacity = get_solar_capacity();

    // read digital values
    load1_call = get_load1();
    load2_call = get_load2();
    load3_call = get_load3();
}

void write_outputs() {

    // set output values
    set_load1(load1);
    set_load2(load2);
    set_load3(load3);

    set_battery_state(battery_state);

    uint8_t value = MAINS_CONST * mains_request;
    set_mains_request(value);
}

void splash_screen() {

    rectangle a;

    a.left = 45;
    a.right = 150;
    a.top = 270;
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