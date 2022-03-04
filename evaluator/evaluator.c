#include <stdio.h>
#include "testprofiles.h"

float score[10];
float scoreTotal = 0;
int hour;

short int CBattery = 0;
short int DBattery = 0;
float MainsReq = 0;

float evaluate(struct test a);
int algorithm(struct test a, int i);

int main()
{
    for (int i = 0; i < 10; i++)
    {
        printf("Test %d\n", i+1);
        score[i] = evaluate(Tests[i]);
        scoreTotal += score[i];
        printf("Score: %2.2f%%\n\n", score[i]);
    }

    float scoreAvg = scoreTotal / 10;
    printf("Average score for all tests: %2.2f%%", scoreAvg);
}

float evaluate(struct test a)
{
    hour = 0;
    for (int i = 0; i < 24; i++)
    {
        a.HousesSaved -= algorithm(a, i);
        a.MainsDraw += MainsReq;
        a.BatteryIntegral += CBattery - DBattery;
    }
    float houseScore = (a.HousesSaved) / 2.5;
    float mainsScore = (240 - a.MainsDraw) / 2.4;

    printf("Houses Saved: %d, Houses Score: %2.2f%%\n", a.HousesSaved, houseScore);
    printf("Mains Draw: %2.1f, Mains Score: %2.2f%%\n", a.MainsDraw/5, mainsScore);

        float batteryScore;
        
        if (a.BatteryIntegral < 0)
        {
            batteryScore = 0;
        }
        else
        {
            batteryScore = (24 - a.BatteryIntegral) / 0.24;
        }

    printf("Battery Integral: %d, Battery Score: %2.2f%%\n", a.BatteryIntegral, batteryScore);

    return (batteryScore / 100.0) * (35.0 / 65.0 * houseScore + 30.0 / 65.0 * mainsScore);
}

int algorithm(struct test a, int i)
{
    int housesLost =0; 

    CBattery = 0;
    DBattery = 0;
    MainsReq = 0;

    float PowerRequired = a.Load1[i] + a.Load2[i] + a.Load3[i];
    float PowerDeficit = PowerRequired - a.Wind[i] - a.PV[i];

    if (PowerDeficit < -1)
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 0;
    }
    else if (PowerDeficit < 0)
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit + CBattery);
    }
        else if (PowerDeficit <=1 && ((hour >= 22 && a.BatteryIntegral < 0) || (hour <=7 && a.BatteryIntegral < 3)))
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit + CBattery);
    }
        else if (PowerDeficit <= 1.8 && ((hour >= 22 && a.BatteryIntegral < 0) || (hour <=7 && a.BatteryIntegral < 3)))
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit + CBattery -0.8);
        housesLost = 1;
    }
    else if (PowerDeficit <= 3 && ((hour >= 22 && a.BatteryIntegral < 0) || (hour <=7 && a.BatteryIntegral < 3)))
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit + CBattery -2);
        housesLost = 3;
    }
    else if (PowerDeficit <= 2 && a.BatteryIntegral <= -2)
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit);
    }
    else if (PowerDeficit <= 3 && a.BatteryIntegral > -2)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery);
    }
    else if (PowerDeficit <= 2.8 && a.BatteryIntegral <= -2)
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit - 0.8);
        housesLost = 1;
    }
    else if (PowerDeficit <= 3.8 && a.BatteryIntegral > -2)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery - 0.8);
        housesLost = 1;
    }
    else if (PowerDeficit <= 4 && a.BatteryIntegral > -2)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery - 2);
        housesLost = 3;
    }
    else if (PowerDeficit <= 4 && a.BatteryIntegral <= -2)
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 5 * (PowerDeficit - 2);
        housesLost = 3;
    }
    else
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 10;
        if (PowerDeficit > 2)
        {
            housesLost = 4;
        }
    }
    hour++;
    return housesLost;
}