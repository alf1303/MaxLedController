#include <stdio.h>
#include <stdlib.h>

struct settings_t {
    int age;
    int weight;
};

settings_t set = {
    5,
    12
};

class FX{
    public:
    settings_t *settings;

    FX(settings_t *sett) {
        settings = sett;
    }
};

int main() {
    printf("main: %p\n", &set);
    FX fx = FX(&set);
    printf("fx: %p\n", fx.settings);
    printf("set.age: %d\n", set.age);
    printf("fx.age: %d\n", fx.settings->age);
    set.age = 111;
    printf("fx.age: %d\n", fx.settings->age);

}