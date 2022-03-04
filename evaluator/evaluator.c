#include <stdio.h>
#include "testprofiles.h"

float score[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float scoreTotal = 0;

short int CBattery = 0;
short int DBattery = 0;
float MainsReq = 0;

float evaluate(struct test a);
void algorithm(struct test a, int i);

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
    for (int i = 0; i < 24; i++)
    {
        algorithm(a, i);
        a.MainsDraw += MainsReq;
        a.BatteryIntegral += CBattery - DBattery;
    }
    float houseScore = (a.HousesSaved) / 2.5;
    float mainsScore = (240 - a.MainsDraw) / 2.4;
    float batteryScore = (24 - a.BatteryIntegral) / 0.24;

    printf("Houses Saved: %d, Houses Score: %2.2f%%\n", a.HousesSaved, houseScore);
    printf("Mains Draw: %2.1f, Mains Score: %2.2f%%\n", a.MainsDraw/5, mainsScore);
    printf("Battery Integral: %d, Battery Score: %2.2f%%\n", a.MainsDraw, mainsScore);

    return batteryScore / 100 * (35 / 65 * houseScore + 30 / 65 * mainsScore);
}

void algorithm(struct test a, int i)
{
    CBattery = 0;
    DBattery = 0;
    MainsReq = 0;

    float PowerRequired = a.Load1[i] + a.Load2[i] + a.Load3[i];
    float PowerDeficit = PowerRequired - a.Wind[i] - a.PV[i];

    if (PowerDeficit < 0)
    {
        CBattery = 1;
        DBattery = 0;
        MainsReq = 0;
    }
    else if (PowerDeficit <= 1)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 0;
    }
    else if (PowerDeficit <= 3)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery);
    }
    else if (PowerDeficit <= 3.8)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery - 0.8);
        a.HousesSaved -= 1;
    }
    else if (PowerDeficit <= 4)
    {
        CBattery = 0;
        DBattery = 1;
        MainsReq = 5 * (PowerDeficit - DBattery - 2);
        a.HousesSaved -= 3;
    }
    else
    {
        CBattery = 0;
        DBattery = 0;
        MainsReq = 10;
        if (PowerDeficit > 2)
        {
            a.HousesSaved -= 4;
        }
    }
}