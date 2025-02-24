#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "ws2812.pio.h"


// TODO
// terminar de comentar

ssd1306_t ssd;

// intensidade dos leds
uint8_t led_r = 5; // Intensidade do vermelho
uint8_t led_g = 5; // Intensidade do verde
uint8_t led_b = 5; // Intensidade do azul

// inputs
#define JOY_PIN 27 // Pino Y joystick
#define BUTTON_A 5 // Simula o sensor de presença
#define BUTTON_B 6 // Simula o sensor que detecta se a janela está fechada

// output
#define WS2812_PIN 7 // Matriz de leds
#define NUM_PIXELS 25
#define BLUE_LED 12

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define IS_RGBW false

volatile uint32_t last_button_a_time = 0;
volatile uint32_t last_button_b_time = 0;

bool presence_sensor_state = true;
bool window_sensor_state = true;
bool temp_sensor_state = true;

const uint32_t debounce_time = 200; // Tempo de debounce em ms

// Buffer para armazenar quais LEDs estão ligados matriz 5x5
bool led_buffer_green[NUM_PIXELS] = {
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0
};

bool led_buffer_blue[NUM_PIXELS] = {
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 0, 0
};

bool led_buffer_red[NUM_PIXELS] = {
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0
};

// Função para enviar um pixel para a matriz de leds
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função para criar um pixel com as cores RGB
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função para atualizar a matriz de leds
void update_led_matrix() {
    // Cria um array com as cores dos pixels
    uint32_t colors[NUM_PIXELS];
    // Preenche o array com as cores dos pixels
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (led_buffer_green[i]) {
            colors[i] = urgb_u32(0, 5, 0); // Verde
        } else if (led_buffer_blue[i]) {
            colors[i] = urgb_u32(0, 0, 5); // Azul
        } else if (led_buffer_red[i]) {
            colors[i] = urgb_u32(5, 0, 0); // Vermelho
        } else {
            colors[i] = urgb_u32(0, 0, 0); // LED apagado
        }
    }
    // Envia os pixels para a matriz
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(colors[i]); // Envia todas as cores corretamente
    }
}

// Função de interrupção para os botões
void button_irq_handler(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    // Verifica se o botão foi pressionado e se o tempo de debounce foi respeitado
    if (gpio == BUTTON_A && (now - last_button_a_time) > debounce_time) {
        presence_sensor_state = !presence_sensor_state; // Inverte o estado do sensor de presença
        led_buffer_green[12] = presence_sensor_state; // Exemplo: LED do meio
        last_button_a_time = now; // Atualiza o tempo do último clique
    }
    // Verifica se o botão foi pressionado e se o tempo de debounce foi respeitado
    if (gpio == BUTTON_B && (now - last_button_b_time) > debounce_time) {
        window_sensor_state = !window_sensor_state; // Inverte o estado do sensor de janela
        led_buffer_blue[22] = window_sensor_state; // Exemplo: Outro LED
        last_button_b_time = now; // Atualiza o tempo do último clique
    }

    update_led_matrix(); // Atualiza a matriz após qualquer mudança
}

float convert_adc_to_temperature(uint16_t adc_value) {
    return ((adc_value - 2048) / 2047.0) * 100.0;
}

int main(){
    
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    stdio_init_all();

    adc_init();
    adc_gpio_init(JOY_PIN);
    adc_select_input(0);

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

    // Inicia o LED azul
    gpio_init(BLUE_LED);
    gpio_set_dir(BLUE_LED, GPIO_OUT);
    gpio_put(BLUE_LED, 0);


    // Interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_irq_handler);

    update_led_matrix(); // Atualiza a matriz de leds
    while (true){
        uint16_t joy_value = adc_read();
        float temperature = convert_adc_to_temperature(joy_value);
        printf("Temperatura: %.2f\n", temperature);

        if(temperature < 27){
            temp_sensor_state = true;
        } else {
            temp_sensor_state = false;
        }

        led_buffer_red[17] = temp_sensor_state;
        update_led_matrix();
        ssd1306_fill(&ssd, false);
        char temp_str[20];
        snprintf(temp_str, sizeof(temp_str), "Temp: %.1fC", temperature);
        ssd1306_draw_string(&ssd, temp_str, 10, 10);

        if (temp_sensor_state && !window_sensor_state) {
            ssd1306_draw_string(&ssd, "Abra a janela", 10, 30);
            gpio_put(BLUE_LED, 0);
        } else if (!temp_sensor_state && window_sensor_state){
            ssd1306_draw_string(&ssd, "Feche a janela", 10, 30);
            gpio_put(BLUE_LED, 0);
        } else if (!temp_sensor_state && !window_sensor_state && !presence_sensor_state){
            gpio_put(BLUE_LED, 1);
        } else {
            gpio_put(BLUE_LED, 0);
        }

        ssd1306_send_data(&ssd);
        sleep_ms(500);
    }
}
