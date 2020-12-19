#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <settings.h>

class FxController{
    public:
    settings_t *sett;
    NeoPixelAnimator animations{2};////
    NeoPixelAnimator animations2{2};////
    RgbColor *fxData;////
    double *rgbData;////
    RgbTemp_t *fxTemp;////
    RgbTemp_t *attackTemp;////
    bool fxRunning;
    bool periodChanged;
    bool speedChanged;
    bool needRecalculate;
    uint8_t previousFxNum;
    int8_t moveDir;
    uint16_t lastPixel;
    int prevIndex;
    int rndShouldGo;
    long tick;
    int fps;

    FxController() {
        fxRunning = false;
        periodChanged = false;
        speedChanged = false;
        needRecalculate = true;
        previousFxNum = 0;
        moveDir = 1;
        rndShouldGo = -1;
        lastPixel = 0;
        prevIndex = -1;
        tick = 0;
        fps = 30;
    }

    void setSettings(settings_t *set) {
        sett = set;
    }

    void clearFxData() {
        printf("LOG: Clearing fxData\n");
        //printf("pixCount: %d\n", sett->pixelCount);
        for(int i = 0; i < sett->pixelCount; i++) {
            fxData[i] = RgbColor(0, 0, 0);
            fxTemp[i] = RgbTemp_t(0, 0, 0);
            rgbData[i] = 0;
        }
        printf("LOG: cleared\n");
    }

    void initFxData() {
          printf("LOG: init fXData\n");
        printf("pixCount: %d\n", sett->pixelCount);
        fxData = new RgbColor[sett->pixelCount]();
        fxTemp = new RgbTemp_t[sett->pixelCount];
        rgbData = new double[sett->pixelCount];
        attackTemp = new RgbTemp_t[sett->pixelCount];
        clearFxData();
                  printf("LOG: inited fxData\n");
    }

    void recalculateTempRGB() {
        for(int i = 0; i < sett->pixelCount; i++) {
            rgbData[sett->pixelCount - i - 1] = (int)(i*(360.0/sett->pixelCount)*normToDouble(sett->fxParts, PARTS_MIN_INT, PARTS_MAX_INT, PARTS_MIN_DOUBLE, PARTS_MAX_DOUBLE))%360;
            //printf("rgbData: i: %d, %f\n", i, rgbData[i]);
        }
        needRecalculate = false;
        //printf("%f, %f, %f\n", fxTemp[0].R, fxTemp[1].R, fxTemp[2].R);
    }

    void recalculateTemp() {
    for(int i = 0; i < sett->pixelCount; i++) {
        double p;
        if(sett->fxParts == 1) {
        p = i*(sett->fxSpread*PIP/sett->pixelCount);
        }
        else {
        p = (i%sett->fxParts)*(sett->fxSpread*PIP/(sett->fxParts-1));
        }

        if(sett->fxSymm) {
        if(sett->fxReverse) {
        fxTemp[i] = RgbTemp_t(p, p, p);
        fxTemp[sett->pixelCount - i - 1] = RgbTemp_t(p, p, p);
        }
        else {
        fxTemp[sett->pixelCount - i - 1] = RgbTemp_t(p, p, p);
        fxTemp[i] = RgbTemp_t(p, p, p);

        }
        }
        else {
        if(sett->fxReverse) {
        fxTemp[i] = RgbTemp_t(p, p, p);
        }
        else {
        fxTemp[sett->pixelCount - i - 1] = RgbTemp_t(p, p, p);
        }
        }
    }
    needRecalculate = false;
    //printf("%f, %f, %f\n", fxTemp[0].R, fxTemp[1].R, fxTemp[2].R);
    }

    void fadeAll() {
        for(uint16_t i = 0; i < sett->pixelCount; i++) {
            fxData[i].Darken(sett->fxWidth);
        }
    }

    double normToDouble(uint8_t val, uint8_t inMin, uint8_t inMax, double outMin, double outMax) {
        double result;
        double scale = (outMax - outMin)/(inMax - inMin);
        if((outMin == 0 && inMin == 0) || (outMin != 0 && inMin != 0)) {
            result = val*scale;
        }
        else if(inMin == 0) {
            result = val*scale + outMin;
        }
        else if(outMin == 0) {
            result = val*scale + outMax;
        }
        return result;
    }
};