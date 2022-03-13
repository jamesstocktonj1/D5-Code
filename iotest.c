#include <avr/io.h>
#include <util/delay.h>
#include "io.h"
#include "lib/lcd.h"
#include <stdlib.h>


int main() {

    //initialise pins
    init_pins();

    init_lcd();
    clear_screen();

    uint16_t i;
    char temp[10];


    while(1) {

        //create saw wave
        for(i=0; i<16; i++) {

            //write value to resistor DAC
            set_mains_request(i);

            clear_screen();
            itoa(i, temp, 10);
            display_string("Value: ");
            display_string(temp);

            _delay_ms(5000);
        }
    }
}