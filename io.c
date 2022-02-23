#include "io.h"





void init_pins() {
    int i;

    //set pins 0-7 on port C to outputs
    for(i=0; i<8; i++) {
        DDRC |= _BV(i);
    }
}


void init_adc() {

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


uint16_t get_mains_capacity() {

    return read_adc(MAINS_CAPACITY);
}

uint16_t get_busbar_voltage() {

    return read_adc(BUSBAR_VOLTAGE);
}

uint16_t get_busbar_current() {

    return read_adc(BUSBAR_CURRENT);
}

uint16_t get_wind_capacity() {

    return read_adc(WIND_CAPACITY);
}

uint16_t get_solar_capacity() {

    return read_adc(SOLAR_CAPACITY);
}


void set_battery_state(battery state) {

    //set charge and discharge lines depending on battery state
    switch(state) {

        case CHARGING:
            set_battery_charge(1);
            set_battery_discharge(0);
            break;

        case DISCHARGING:
            set_battery_charge(0);
            set_battery_discharge(1);

        default:
            set_battery_charge(0);
            set_battery_discharge(0);
    }
}


void set_battery_charge(uint8_t state) {

    if(state) {
        PORTC |= _BV(CHARGE_BATTERY);
    }
    else {
        PORTC &= ~_BV(CHARGE_BATTERY);
    }
}

void set_battery_discharge(uint8_t state) {

    if(state) {
        PORTC |= _BV(DISCHARGE_BATTERY);
    }
    else {
        PORTC &= ~_BV(DISCHARGE_BATTERY);
    }
}

void set_load1(uint8_t state) {

    if(state) {
        PORTC |= _BV(SWITCH_LOAD1);
    }
    else {
        PORTC &= ~_BV(SWITCH_LOAD1);
    }
}

void set_load2(uint8_t state) {

    if(state) {
        PORTC |= _BV(SWITCH_LOAD2);
    }
    else {
        PORTC &= ~_BV(SWITCH_LOAD2);
    }
}

void set_load3(uint8_t state) {

    if(state) {
        PORTC |= _BV(SWITCH_LOAD3);
    }
    else {
        PORTC &= ~_BV(SWITCH_LOAD3);
    }
}


uint8_t get_load1(void) {

    return (PINC & _BV(CALL_LOAD1));
}

uint8_t get_load2(void) {

    return (PINC & _BV(CALL_LOAD2));
}

uint8_t get_load3(void) {

    return (PINC & _BV(CALL_LOAD3));
}
