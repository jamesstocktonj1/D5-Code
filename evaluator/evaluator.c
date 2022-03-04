#include <stdio.h>
#include "testprofiles.h"

struct test TestString[10] = {Test1, Test2, Test3, Test4, Test5, Test6, Test7, Test8, Test9, TestWorse};
float score[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float scoreTotal = 0;

short int CBattery = 0;
short int DBattery = 0;
float MainsReq = 0;

float evaluate(struct test a);
void algorithm(struct test a, int i);

int main()
{
    for (i = 0; i < 10; i++)
    {
        score[i] = evaluate(TestString[i]);
        scoreTotal += score[i];
    }

    float scoreAvg = scoreTotal/10;
    printf("Average score for all tests: %2.2f%%", scoreAvg);
}

float evaluate(struct test a)
{
    for (i = 0; i < 24; i++)
    {
        algorithm(a, i);
        a.MainsDraw += MainsReq;
        a.BatteryIntegral += CBattery - DBattery;
    }
    float houseScore = (250 - a.HousesSaved)/250;
    float mainsScore = (48 - a.MainsDraw)/48;
    float batteryScore = (24 - a.BatteryIntegral)/24;

    printf("Houses Saved: %d, Houses Score: %2.2f%%\\", a.HousesSaved, houseScore);
    printf("Mains Draw: %2.1f, Mains Score: %2.2f%%\\", a.MainsDraw, mainsScore);
    printf("Battery Integral: %d, Battery Score: %2.2f%%\\", a.MainsDraw, mainsScore);

    float score = batteryScore/100 * (35/65 * houseScore + 30/65 * mainsScore);

    printf()
}

void algorithm(struct test a, int i)
{
    CBattery = 0;
    DBattery = 0;
    MainsReq = 0;

    float PowerRequired = a.Load1[i] + a.Load2[i] + a.Load3[i];
    float PowerDeficit = PowerRequired - a.Wind[i] - a.PV[i];

    switch(PowerDeficit)
    {
        case(PowerDeficit < 0):
            CBattery = 1;
            DBattery = 0;
            MainsReq = 0;
        break;    
        case(PowerDeficit <= 1):
            CBattery = 0;
            DBattery = 1;
            MainsReq = 0;
        break; 
        case(PowerDeficit <= 3):
            CBattery = 0;
            DBattery = 1;
            MainsReq = 5*(PowerDeficit - DBattery);
        break; 
        case(PowerDeficit <= 3.8):
            CBattery = 0;
            DBattery = 1;
            MainsReq = 5*(PowerDeficit - DBattery - 0.8);
            a.HousesSaved -= 1;
        break; 
        case(PowerDeficit <= 4):
            CBattery = 0;
            DBattery = 1;
            MainsReq = 5*(PowerDeficit - DBattery - 2);
            a.HousesSaved -= 3;
        break; 
        default:
            CBattery = 0;
            DBattery = 0;
            MainsReq = 10;
            if(PowerDeficit > 2)
            {
                a.HousesSaved -= 4;
            }
        break; 
    }
}