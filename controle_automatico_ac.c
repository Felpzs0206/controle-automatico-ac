#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

#define JOY_Y // Simula a temperatura
#define BUTTON_A // Simula o sensor de presença
#define BUTTON_B // Simula o sensor que detecta se a janela está fechada

int main()
{
    stdio_init_all();

    while (true){
        sleep_ms(1000);
    }
}
