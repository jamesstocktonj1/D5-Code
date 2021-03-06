#ifndef IO_H
#define IO_H

#include <avr/io.h>


//pin definitions
//PORTA
#define BUSBAR_VOLTAGE 0
#define BUSBAR_CURRENT 1
#define WIND_CAPACITY 2
#define SOLAR_CAPACITY 3
#define MAINS_REQ0 4
#define MAINS_REQ1 5
#define MAINS_REQ2 6
#define MAINS_REQ3 7

//PORTC
#define CHARGE_BATTERY 1
#define DISCHARGE_BATTERY 0
#define SWITCH_LOAD1 4
#define SWITCH_LOAD2 3
#define SWITCH_LOAD3 2
#define CALL_LOAD1 7
#define CALL_LOAD2 6
#define CALL_LOAD3 5

//initialise digital pins
void init_pins(void);

//initialise analog pins
void init_adc(void);
uint16_t read_adc(uint8_t channel);

//analog input functions
//uint16_t get_mains_capacity(void);
uint16_t get_busbar_voltage(void);
uint16_t get_busbar_current(void);
uint16_t get_wind_capacity(void);
uint16_t get_solar_capacity(void);

//digital output functions
void set_mains_request(uint16_t value);

//battery functions
typedef enum {
    CHARGING, 
    DISCHARGING, 
    DISCONNECTED
} battery;
void set_battery_state(battery state);

//digital output functions
void set_battery_charge(uint8_t state);
void set_battery_discharge(uint8_t state);
void set_load1(uint8_t state);
void set_load2(uint8_t state);
void set_load3(uint8_t state);

//digital input functions
uint8_t get_load1(void);
uint8_t get_load2(void);
uint8_t get_load3(void);

#endif