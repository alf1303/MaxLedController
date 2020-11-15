class FxController{
    public:
    bool fxRunning;
    bool periodChanged;
    bool needRecalculate;
    uint8_t previousFxNum;
    int fps;

    FxController() {
        fxRunning = false;
        periodChanged = false;
        needRecalculate = true;
        previousFxNum = 0;
        fps = 30;
    }
};