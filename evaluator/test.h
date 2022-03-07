#ifndef TEST_H
#define TEST_H

typedef struct test {
    float Wind[24];
    float PV[24];
    float Load1[24];
    float Load2[24];
    float Load3[24];

    int HousesSaved;
    float MainsDraw;
    int BatteryIntegral;
} test;

#endif