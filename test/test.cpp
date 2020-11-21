#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main() {
    uint16_t kj = 2;
    float ff = 2.123;
    float fff = kj*ff;
    printf("***************: %f\n", fff);
    uint8_t params;
    params = 255;
    printf("%d\n", params);
    printf("%d, %d, %d, %d, %d, %d, %d, %d\n", (params>>7)&1, (params>>6)&1, (params>>5)&1, (params>>4)&1, (params>>3)&1, (params>>2)&1, (params>>1)&1, (params)&1);
    params &= ~(1<<3);
    printf("%d, %d, %d, %d, %d, %d, %d, %d\n", (params>>7)&1, (params>>6)&1, (params>>5)&1, (params>>4)&1, (params>>3)&1, (params>>2)&1, (params>>1)&1, (params)&1);

}

void randdd() {
    const char* ssid = "MaxLedNet";
    srand(25);
    int num = rand()%6000;
    char rndStr[32];
    sprintf(rndStr, "%d", num);
    char temp[32] = "MaxLedNet";
    strcat(temp, rndStr);
    printf("%s\n",temp);
}