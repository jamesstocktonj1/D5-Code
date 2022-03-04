typedef struct test
{
    float Wind[24];
    float PV[24];
    float Load1[24];
    float Load2[24];
    float Load3[24];

    int HousesSaved = 250;
    float MainsDraw = 0;
    int BatteryIntegral = 0;
};