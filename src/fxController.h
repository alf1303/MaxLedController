class FxController{
    public:
    bool fxRunning;
    bool periodChanged;
    bool speedChanged;
    bool needRecalculate;
    uint8_t previousFxNum;
    int8_t moveDir;
    uint16_t lastPixel;
    int prevIndex;
    long tick;
    int fps;

    FxController() {
        fxRunning = false;
        periodChanged = false;
        speedChanged = false;
        needRecalculate = true;
        previousFxNum = 0;
        moveDir = 1;
        prevIndex = -1;
        lastPixel = 1;
        tick = 0;
        fps = 30;
    }
};