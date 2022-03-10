#include <stdio.h>
#include "testprofiles.h"
#include "test.h"

float score[10];
float scoreTotal = 0;
int hour;

float V2_2 = 0;
float V2_5 = 0;

short int CBattery = 0;
short int DBattery = 0;
float MainsReq = 0;

float evaluate(struct test a);
int algorithm(struct test a, int i);

int main()
{
    // Run all Tests
    for (int i = 0; i < 10; i++)
    {
        printf("Test %d\n", i + 1);
        score[i] = evaluate(Tests[i]);
        scoreTotal += score[i];
    }

    // Print average score
    float scoreAvg = scoreTotal / 10;
    printf("Average score for all tests: %2.2f%%", scoreAvg);
}

float evaluate(struct test a)
{
    // Reset hour for new test
    hour = 0;

    // For each hour check the algorithm and record scoring metrics
    for (hour = 0; hour < 24; hour++)
    {
        a.HousesSaved -= algorithm(a, hour);
        a.MainsDraw += MainsReq;
        a.BatteryIntegral += CBattery - DBattery;
    }

    // Calculate scores
    float houseScore = (a.HousesSaved) / 2.5;
    float mainsScore = (240 - a.MainsDraw) / 2.4;
    float batteryScore;

    if (a.BatteryIntegral < 0)
    {
        batteryScore = 0;
    }
    else
    {
        batteryScore = (24 - a.BatteryIntegral) / 0.24;
    }

    float combinedScore = (batteryScore / 100.0) * (35.0 / 65.0 * houseScore + 30.0 / 65.0 * mainsScore);

    // Print Scores
    printf("Houses Saved: %d, Houses Score: %2.2f%%\n", a.HousesSaved, houseScore);
    printf("Mains Draw: %2.1f, Mains Score: %2.2f%%\n", a.MainsDraw / 5, mainsScore);
    printf("Battery Integral: %d, Battery Score: %2.2f%%\n", a.BatteryIntegral, batteryScore);
    printf("Score: %2.2f%%\n\n", combinedScore);

    // Return percentage to allow for average score calculation
    return combinedScore;
}

// Main algorithm to be implemented in project
// Average score of 79.16%
int algorithm(struct test a, int i)
{
    // Set constants to zero
    int housesLost = 0;
    CBattery = 0;
    DBattery = 0;
    MainsReq = 0;

    // Calculate power necessary
    float PowerRequired = a.Load1[i] + a.Load2[i] + a.Load3[i];
    float PowerDeficit = PowerRequired - a.Wind[i] - a.PV[i];

    float mainsSupply = (V2_2 - V2_5) - a.Wind[i] - a.PV[i];

    // Charge battery if excess renewable
    if (PowerDeficit < -1)
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 0;
    }

    // Charge battery if some spare renewable and make up the difference with mains
    // Change to -0.1 increases score but is probably specific to these tests
    else if (-1 <= PowerDeficit && PowerDeficit < 0)
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit + CBattery);
    }

    // Offpeak

    // Charge battery if small load and offpeak
    // Charge to +2 before peak
    // Charge to 0 after peak
    else if ((0 <= PowerDeficit && PowerDeficit < 1) && ((hour <= 7 && a.BatteryIntegral <= 2) || (hour >= 22 && a.BatteryIntegral < 0)))
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit + CBattery);
    }

    // Charge battery, switch off lights if medium load and offpeak
    else if ((1 <= PowerDeficit && PowerDeficit < 1.8) && ((hour <= 7 && a.BatteryIntegral <= 2) || (hour >= 22 && a.BatteryIntegral < 0)))
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit + CBattery - 0.8);
        housesLost = 1;
    }

    // Charge battery, switch off lifters if high load and offpeak
    else if ((1.8 <= PowerDeficit && PowerDeficit < 3) && ((hour <= 7 && a.BatteryIntegral <= 2) || (hour >= 22 && a.BatteryIntegral < 0)))
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit + CBattery - 2);
        housesLost = 3;
    }

    // Battery at -2

    // Small load, supply with mains
    else if ((0 <= PowerDeficit && PowerDeficit < 2) && a.BatteryIntegral <= -2)
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit);
    }

    // Medium load, switch off lights supply with mains
    else if ((2 <= PowerDeficit && PowerDeficit < 2.8) && a.BatteryIntegral <= -2)
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit - 0.8);
        housesLost = 1;
    }

    // High load, switch off lifters supply with mains
    else if ((2.8 <= PowerDeficit && PowerDeficit <= 4) && a.BatteryIntegral <= -2)
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit - 2);
        housesLost = 3;
    }

    // Battery above -2

    // Small load, discharge battery and supply with mains
    else if ((0 <= PowerDeficit && PowerDeficit < 3) && a.BatteryIntegral > -2)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery);
    }

    // Medium load, switch off lights discharge battery and supply with mains
    else if ((3 <= PowerDeficit && PowerDeficit < 3.8) && a.BatteryIntegral > -2)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery - 0.8);
        housesLost = 1;
    }

    // High load, switch off lifters dischage battery and supply with mains
    else if ((3.8 <= PowerDeficit && PowerDeficit <= 4) && a.BatteryIntegral > -2)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery - 2);
        housesLost = 3;
    }

    // If unsure, switch off lifters dont make battery better or worse, supply with mains
    else
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 10;
        
        if (PowerDeficit > 2) {
            housesLost = 4;
        }
    }

    return housesLost;
}