#pragma once
#define SPEED_MIN_INT 0
#define SPEED_MAX_INT 100
#define SPEED_MIN_DOUBLE 0.0
#define SPEED_MAX_DOUBLE 10.0
#define WIDTH_MIN_INT 1
#define WIDTH_MAX_INT 100
#define WIDTH_MIN_DOUBLE 0.8
#define WIDTH_MAX_DOUBLE 3.0
#define PARTS_MIN_INT 1
#define PARTS_MAX_INT 100
#define PARTS_MIN_DOUBLE 0.1
#define PARTS_MAX_DOUBLE 5

#define PIP PI

//main settings type
typedef struct {
    char* name;
    uint8_t netMode; //0 -0 standalone, 1 - client
    uint16_t pixelCount;
    char* network;
    char* password;
    uint8_t playlistSize;
    bool playlistMode;
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
    uint16_t startPixel;
    uint16_t endPixel;
    boolean fxReverse;
    boolean fxAttack;
    boolean fxSymm;
    boolean fxRnd;
    boolean fxRndColor;
} settings_t;

//playlist item setting type
typedef struct {
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
    boolean fxReverse;
    boolean fxAttack;
    boolean fxSymm;
    boolean fxRnd;
    boolean fxRndColor;
} ledsettings_t;

struct RgbTemp_t{
    double R;
    double G;
    double B;

    RgbTemp_t(double r, double g, double b):
        R(r), G(g), B(b) {};

    RgbTemp_t() {};
};