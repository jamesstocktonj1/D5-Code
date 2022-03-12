

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


void reduceLoad(uint16_t powerDeficit) {

    uint16_t makeUpPower = powerDeficit - MAINS_MAX;

    //switch off smallest load first (load 3)
    if(makeUpPower <= LOAD3_MAX) {
        load1 = load1_call;
        load2 = load2_call;
        load3 = 0;

        powerDeficit -= LOAD3_MAX;
    }

    //switch off load 2 to lose the least amount of marks
    else if(LOAD3_MAX < makeUpPower <= LOAD2_MAX) {
        load1 = load1_call;
        load2 = 0;
        load3 = load1_call;

        powerDeficit -=  LOAD1_MAX;
    }

    //switch off load 3 and 2 if cant keep up
    else if(LOAD2_MAX < makeUpPower <= (LOAD2_MAX + LOAD3_MAX)) {
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
}


// Main algorithm to be implemented in project
// Average score of 79.16%
void algorithm(void) {

    //store current time
    uint32_t current_time = millis_timer;

    busbar_current = get_busbar_current();

    //calculate difference between actual busbar current and what was requested (previous values)
    uint16_t mainsDeficit = (mains_request == 0) ? 0 : busbar_current - (mains_request + wind_capacity + solar_capacity);
    uint16_t actualMainsCapacity = mains_request - mainsDeficit;

    //decide if off peak time (batter should charge)
    //uint8_t offpeak = ((current_time <= (8 * 60 * 1000) && battery_charge <= 2) || (current_time >= (22 * 60 * 1000) && battery_charge < 0));

    //read in new values
    read_inputs();

    //calculate required load and sum of renewable sources
    uint16_t loadSum = load1_call + load2_call + load3_call + mainsDeficit;
    uint16_t renewableSum = wind_capacity + solar_capacity;

    //calculate power required (excess power if negative)
    int16_t powerDeficit = loadSum - renewableSum;
    uint16_t powerExcess = (powerDeficit < 0) ? (powerDeficit * -1) : 0;




    //make decisions on battery status

    //charging if:
    //time < 7 and charge < 2
    //time > 22 and charge < 0
    //spare renewables
    if((current_time < 7 && battery_charge < 2) || (current_time > 22 && battery_charge < 2) || (powerExcess > 0)) {
        battery_status = CHARGING;
        powerDeficit += AMP;
    }
    
    //discharging if:
    //deficit < 1 and charge > -2
    //t > 22 and charge > 1
    else if((powerDeficit > AMP && battery_charge > -2) || (current_time > 22 && battery_charge > 0.25)) {
        batter_status = DISCHARGING;
        powerDeficit -= AMP;
    }

    //else disconnect
    else {
        battery_status = DISCONNECTED;
    }

    //make decisions on mains status

    if(powerDeficit )
    //if mains can supply remaining power
    if(powerDeficit <= MAINS_MAX) {
        mains_request = powerDeficit;
    }

    //if cant supply remaining power then reduce loads
    else {
        reduceLoad(powerDeficit);

        uint16_t makeUpPower = powerDeficit - MAINS_MAX;

        //switch off smallest load first (load 3)
        if(makeUpPower <= LOAD3_MAX) {
            load1 = load1_call;
            load2 = load2_call;
            load3 = 0;

            powerDeficit -= LOAD3_MAX;
        }

        //switch off load 2 to lose the least amount of marks
        else if(LOAD3_MAX < makeUpPower <= LOAD2_MAX) {
            load1 = load1_call;
            load2 = 0;
            load3 = load1_call;

            powerDeficit -=  LOAD1_MAX;
        }

        //switch off load 3 and 2 if cant keep up
        else if(LOAD2_MAX < makeUpPower <= (LOAD2_MAX + LOAD3_MAX)) {
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
    }
}