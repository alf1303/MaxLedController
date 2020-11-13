#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main() {
    const char* ssid = "MaxLedNet";
    srand(25);
    int num = rand()%6000;
    char rndStr[32];
    sprintf(rndStr, "%d", num);
    char temp[32] = "MaxLedNet";
    strcat(temp, rndStr);
    printf("%s\n",temp);
}