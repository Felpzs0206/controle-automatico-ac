#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "ws2812.pio.h"


// TODO
// 1. LIGAR 2 LEDS DA MATRIZ COM CORES DIFERENTES
// 2. SIMULAR A TEMPERATURA COM O JOYSTICK
// 3. SIMULAR O SENSOR DE PRESENÇA COM O BOTÃO A
// 4. SIMULAR O SENSOR DE JANELA FECHADA COM O BOTÃO B
// 5. LIGAR O LED AZUL DO RGB PARA SIMULAR O AC LIGADO

ssd1306_t ssd;

// intensidade dos leds
uint8_t led_r = 5; // Intensidade do vermelho
uint8_t led_g = 5; // Intensidade do verde
uint8_t led_b = 5; // Intensidade do azul

// inputs
#define JOY_Y // Simula a temperatura
#define BUTTON_A 5 // Simula o sensor de presença
#define BUTTON_B 6 // Simula o sensor que detecta se a janela está fechada

// output
#define WS2812_PIN 7 // Matriz de leds
#define NUM_PIXELS 25

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define IS_RGBW false

volatile uint32_t last_button_a_time = 0;
volatile uint32_t last_button_b_time = 0;

bool presence_sensor_state = true;
bool window_sensor_state = true;

const uint32_t debounce_time = 200; // Tempo de debounce em ms

// Buffer para armazenar quais LEDs estão ligados matriz 5x5
bool led_buffer_green[NUM_PIXELS] = {
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0
};

bool led_buffer_red[NUM_PIXELS] = {
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 0, 0
};

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}


void update_led_matrix() {
    uint32_t colors[NUM_PIXELS];

    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_buffer_green[i]) {
            colors[i] = urgb_u32(0, 5, 0); // Verde
        } else if (led_buffer_red[i]) {
            colors[i] = urgb_u32(5, 0, 0); // Vermelho
        } else {
            colors[i] = urgb_u32(0, 0, 0); // LED apagado
        }
    }

    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(colors[i]); // Envia todas as cores corretamente
    }
}

// Função de interrupção para os botões
void button_irq_handler(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A && (now - last_button_a_time) > debounce_time) {
        presence_sensor_state = !presence_sensor_state;
        led_buffer_green[12] = presence_sensor_state; // Exemplo: LED do meio
        last_button_a_time = now;
    }

    if (gpio == BUTTON_B && (now - last_button_b_time) > debounce_time) {
        window_sensor_state = !window_sensor_state;
        led_buffer_red[22] = window_sensor_state; // Exemplo: Outro LED
        last_button_b_time = now;
    }

    update_led_matrix(); // Atualiza a matriz após qualquer mudança
}

int main(){
    
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicia os botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);



    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);

    update_led_matrix(); // Atualiza a matriz de leds
    while (true){
        sleep_ms(1000);
    }
}
