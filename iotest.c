#include <avr/io.h>
#include <util/delay.h>
#include "io.h"



int main() {

    //initialise pins
    init_pins();

    uint8_t i;


    while(1) {

        //create saw wave
        for(i=0; i<16; i++) {

            //write value to resistor DAC
            set_mains_request(i);

            _delay_ms(100);
        }
    }
}