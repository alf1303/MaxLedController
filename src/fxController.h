class FxController{
    public:
    bool periodChanged;
    bool needRecalculate;
    int fps;

    FxController() {
        periodChanged = false;
        needRecalculate = true;
        fps = 30;
    }
};