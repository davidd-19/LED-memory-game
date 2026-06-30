#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Number of LEDs and buttons in the game
#define LED_COUNT 4

// Number of steps in the memory sequence
#define ROUNDS 4

// Delay times for different difficulty levels (milliseconds)
#define EASY_DELAY        800
#define MEDIUM_DELAY      400
#define HARD_DELAY        200
#define IMPOSSIBLE_DELAY   50   // good luck :) (last min idea)

#define FINAL_LEVEL 3   // impossible level index

// Turns RGB LED on or off
void set_rgb(uint r_pin, uint g_pin, uint b_pin, bool r, bool g, bool b) {
    gpio_put(r_pin, r);
    gpio_put(g_pin, g);
    gpio_put(b_pin, b);
}

// Changes the colors of the RGB LED
void flash_rgb(uint r_pin, uint g_pin, uint b_pin, bool r, bool g, bool b, int ms) {
    set_rgb(r_pin, g_pin, b_pin, r, g, b);
    sleep_ms(ms);
    set_rgb(r_pin, g_pin, b_pin, 0, 0, 0);
}

// Waits until a button is pressed
int wait_for_button(const uint button_pins[]) {
    while (true) {
        for (int i = 0; i < LED_COUNT; i++) {
            if (gpio_get(button_pins[i]) == 0) {
                sleep_ms(200); // debounce
                return i;
            }
        }
    }
}

// Plays the LED sequence
void play_sequence(int sequence[], int length, const uint led_pins[], int delay_ms) {
    for (int i = 0; i < length; i++) {
        gpio_put(led_pins[sequence[i]], 1);
        sleep_ms(delay_ms);
        gpio_put(led_pins[sequence[i]], 0);
        sleep_ms(delay_ms / 2);
    }
}
// sleep_ms(delay_ms / 2) works for the first three difficulties but the last one has an if statement in the main

// Reads the player's input
bool get_player_input(int sequence[], int length, const uint led_pins[], const uint button_pins[]) {
    for (int i = 0; i < length; i++) {
        int pressed = wait_for_button(button_pins);

        gpio_put(led_pins[pressed], 1);
        sleep_ms(120);
        gpio_put(led_pins[pressed], 0);

        if (pressed != sequence[i]) {
            return false;
        }
    }
    return true;
}

int main() {
    stdio_init_all();

    uint led_pins[LED_COUNT] = {2, 3, 4, 5};
    uint button_pins[LED_COUNT] = {6, 7, 8, 9};

    uint rgb_r = 10;
    uint rgb_g = 11;
    uint rgb_b = 12;

    int difficulty = 0; // 0=easy, 1=medium, 2=hard, 3=impossible

    int delays[4] = {
        EASY_DELAY,
        MEDIUM_DELAY,
        HARD_DELAY,
        IMPOSSIBLE_DELAY
    };

    // Initialise LEDs
    for (int i = 0; i < LED_COUNT; i++) {
        gpio_init(led_pins[i]);
        gpio_set_dir(led_pins[i], GPIO_OUT);
    }

    // Initialise buttons
    for (int i = 0; i < LED_COUNT; i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);
    }

    // Initialise RGB LED
    gpio_init(rgb_r);
    gpio_init(rgb_g);
    gpio_init(rgb_b);
    gpio_set_dir(rgb_r, GPIO_OUT);
    gpio_set_dir(rgb_g, GPIO_OUT);
    gpio_set_dir(rgb_b, GPIO_OUT);

    // Without srand the LEDs would follow the same sequence 
    srand(to_us_since_boot(get_absolute_time()));

    while (true) {
        int sequence[ROUNDS];

        for (int i = 0; i < ROUNDS; i++) {
            sequence[i] = rand() % LED_COUNT;
        }

        play_sequence(sequence, ROUNDS, led_pins, delays[difficulty]);

        bool success = get_player_input(sequence, ROUNDS, led_pins, button_pins);

        // RGB feedback
        if (success) {
            flash_rgb(rgb_r, rgb_g, rgb_b, 0, 1, 0, 700);
        } else {
            flash_rgb(rgb_r, rgb_g, rgb_b, 1, 0, 0, 700);
        }

        // Handle difficulty progression and reset
        if (difficulty == FINAL_LEVEL) {
            sleep_ms(15000);   // pause after impossible
            difficulty = 0;    // reset to easy
        } else {
            if (success) {
                difficulty++;
            } else {
                difficulty = 0;
            }
        }

        sleep_ms(1000);
    }
}
