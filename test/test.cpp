#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    uint8_t one;
    uint8_t two;
} set_t;

void func(set_t &s1, set_t &s2) {
    printf("1: one: %d, two: %d *** 2: one: %d, two: %d\n", s1.one, s1.two, s2.one, s2.two);
    s1.one = 2;
    s1.two = 2;
}

int main() {

    int *p1;
    int *p2;
    *p1 = 3;
    p2 = p1;
    printf("%p, %p\n", p1, p2);
    p2++;
    printf("%p, %p\n", p1, p2);
    set_t set = {
        12,
        13
    };

    set_t set2{
        14,
        15
    };
func(set, set2);

printf("%d, %d\n", set.one, set.two);


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