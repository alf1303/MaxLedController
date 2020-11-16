#include <Arduino.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <fxController.h>
#include <settings.h>

#define PIN 2 // pin to connect led strip, ignored for esp8266
#define PIXELCOUNT 10 //default number of pixels in led strip

#define PORT_IN 6454
#define PORT_OUT 6455
#define PORT_OUT_UPD 6457
#define VERSION "v_0.1.0"
#define UNI 10
#define MAIN_FILE "/mainfile"
#define NAME_FILE "/namefile"
#define SSID_FILE "/ssidfile"
#define PASS_FILE "/passfile"

extern bool toAnswer;
extern bool isFading;
extern WiFiUDP wifiUDP;
extern FxController FX;

extern uint8_t hData[5];
extern uint8_t hData1[13];
extern uint8_t hData2[17];
extern char command;
extern char option; 
extern int mask;
extern IPAddress sourceIP;
extern uint8_t uni;


//NeoPixelBus
extern NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip;
extern RgbColor *fxData; //array, containing data of current fx

struct RgbTemp_t{
    double R;
    double G;
    double B;

    RgbTemp_t(double r, double g, double b):
        R(r), G(g), B(b) {};

    RgbTemp_t() {};
};

extern RgbTemp_t *fxTemp;
extern RgbTemp_t *attackTemp;

//main settings type
typedef struct {
    char* name;
    uint8_t netMode; //0 -0 standalone, 1 - client
    int pixelCount;
    char* network;
    char* password;
    uint8_t playlistSize;
    uint8_t dimmer;
    RgbColor color;
    RgbColor fxColor;
    uint8_t strobe;
    uint8_t fxNumber;
    double fxSpeed;
    uint8_t fxSize;
    uint8_t fxParts;
    uint8_t fxFade;
    uint8_t fxParams;
    uint8_t fxSpread;
    uint8_t fxWidth;
    uint8_t startPixel;
    uint8_t endPixel;
    boolean fxReverse;
    boolean fxAttack;
} settings_t;

//playlist item setting type
typedef struct {
    uint8_t dimmer;
    RgbColor color;
    RgbColor fxColor;
    uint8_t strobe;
    uint8_t fxNumber;
    uint8_t fxSpeed;
    uint8_t fxSize;
    uint8_t fxParts;
    uint8_t fxFade;
    uint8_t fxParams;
    uint8_t fxSpread;
    uint8_t fxWidth;
    boolean fxReverse;
    boolean fxAttack;
} ledsettings_t;

extern ledsettings_t *playlist; //array, containing data for settings to be played

extern settings_t settings; //main settings
extern settings_t request; //variable for storing data, received via UDP from controll App

//predefined colors
extern RgbColor black;
extern RgbColor red;
extern RgbColor green;
extern RgbColor blue;
extern RgbColor highliteColor;
extern RgbColor beforeHLColor;
extern bool _highlite;

void OTA_Func(); //For uploading firmware via OTA
void loadSettingsFromFs(); //implement
void saveSettingsToFs(boolean first); //implement
void readUDP(); //reads data from UDP packet
void processRequest();//implment //process data received
void processData(); //implement
void processSetCommand();
void processGetCommand();
void formAnswerInfo(int portOUT);
void setMainSettings();
void setHighliteMode();
void unsetHighliteMode();
void setReset();
void setFormatFS();
void setStaticColor(RgbColor color);
void showStrip();
void setPixelColor(int i, RgbColor color); //wrapper to set sum of color and fx
void outToStrip();
void saveNetworkDataToFs(bool first);
void saveNameToFs(bool first);
void loadNetworkDataFromFs();
void initSettings();
void test2();
void startUdpServer();
void initFxData();
void sinus();
void setRandomSsidName();
void processFx();
void clearFxData();
double speedToDouble(uint8_t speed);
uint8_t speedToInt(double speed);
double widthToDouble(uint8_t parts);
uint8_t widthToInt(double parts);
double normToDouble(uint8_t value, uint8_t inMin, uint8_t inMax, double outMin, double outMax);

void setupAnimations();
void moveAnim(const AnimationParam& param);
void fadeAnim(const AnimationParam& param);
void fadeAll();
extern NeoPixelAnimator animations;
extern uint16_t lastPixel;
extern int8_t moveDir;
