#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define STEP_US 5u

/*
 * SN74HC164N automatic functional tester
 * 8-bit serial-in, parallel-out shift register.
 *
 * Power:
 *   IC pin 14 -> Pico 3V3(OUT), physical pin 36
 *   IC pin 7  -> Pico GND, physical pin 38
 *
 * Pico outputs to DUT:
 *   GP2 (physical 4) -> A,   IC pin 1
 *   GP3 (physical 5) -> B,   IC pin 2
 *   GP4 (physical 6) -> CLK, IC pin 8
 *   GP5 (physical 7) -> CLR, IC pin 9 (active LOW)
 *
 * DUT outputs to Pico:
 *   QA pin 3  -> GP6,  physical 9
 *   QB pin 4  -> GP7,  physical 10
 *   QC pin 5  -> GP8,  physical 11
 *   QD pin 6  -> GP9,  physical 12
 *   QE pin 10 -> GP10, physical 14
 *   QF pin 11 -> GP11, physical 15
 *   QG pin 12 -> GP12, physical 16
 *   QH pin 13 -> GP13, physical 17
 */

static const uint A_GPIO = 2;
static const uint B_GPIO = 3;
static const uint CLK_GPIO = 4;
static const uint CLR_GPIO = 5;
static const uint q_gpios[8] = {6, 7, 8, 9, 10, 11, 12, 13};

static void wait_for_usb(void)
{
    while (!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(250);
}

static void init_output(uint gpio, bool value)
{
    gpio_init(gpio);
    gpio_put(gpio, value);
    gpio_set_dir(gpio, GPIO_OUT);
}

static void init_fixture(void)
{
    init_output(A_GPIO, 0);
    init_output(B_GPIO, 0);
    init_output(CLK_GPIO, 0);
    init_output(CLR_GPIO, 1);

    for (unsigned i = 0; i < 8; ++i) {
        gpio_init(q_gpios[i]);
        gpio_set_dir(q_gpios[i], GPIO_IN);
        gpio_disable_pulls(q_gpios[i]);
    }
}

static void pulse_clock(void)
{
    gpio_put(CLK_GPIO, 0);
    sleep_us(STEP_US);
    gpio_put(CLK_GPIO, 1);
    sleep_us(STEP_US);
    gpio_put(CLK_GPIO, 0);
    sleep_us(STEP_US);
}

static void clear_register(void)
{
    gpio_put(CLR_GPIO, 0);
    sleep_us(STEP_US);
    gpio_put(CLR_GPIO, 1);
    sleep_us(STEP_US);
}

static uint8_t read_parallel(void)
{
    uint8_t value = 0;
    for (unsigned i = 0; i < 8; ++i) {
        if (gpio_get(q_gpios[i])) value |= (uint8_t)(1u << i);
    }
    return value;
}

static void shift_pattern(uint8_t pattern, bool data_on_a)
{
    clear_register();

    if (data_on_a) {
        gpio_put(B_GPIO, 1);
    } else {
        gpio_put(A_GPIO, 1);
    }

    for (int bit = 7; bit >= 0; --bit) {
        bool data = (pattern >> bit) & 1u;
        if (data_on_a) gpio_put(A_GPIO, data);
        else gpio_put(B_GPIO, data);
        pulse_clock();
    }

    gpio_put(A_GPIO, 0);
    gpio_put(B_GPIO, 0);
}

int main(void)
{
    stdio_init_all();
    init_fixture();
    wait_for_usb();

    printf("\n==================================================\n");
    printf("SN74HC164N AUTOMATIC FUNCTIONAL TEST\n");
    printf("8-bit serial-in, parallel-out shift register\n");
    printf("==================================================\n");

    unsigned failures = 0;

    clear_register();
    uint8_t measured = read_parallel();
    bool clear_pass = measured == 0x00u;
    printf("\nCLR test: expected=0x00 measured=0x%02X %s\n",
           measured, clear_pass ? "PASS" : "FAIL");
    if (!clear_pass) ++failures;

    const uint8_t patterns[] = {0x00, 0xFF, 0xA5, 0x5A, 0x81, 0x3C};

    printf("\nPattern | Serial input used | Measured | Result\n");
    printf("--------+-------------------+----------+-------\n");

    for (size_t i = 0; i < ARRAY_COUNT(patterns); ++i) {
        bool use_a = (i % 2u) == 0;
        shift_pattern(patterns[i], use_a);
        measured = read_parallel();
        bool pass = measured == patterns[i];

        printf("  0x%02X  |        %c          |   0x%02X   | %s\n",
               patterns[i], use_a ? 'A' : 'B', measured,
               pass ? "PASS" : "FAIL");
        if (!pass) ++failures;
    }

    clear_register();

    printf("\n==================================================\n");
    printf("FINAL RESULT: %s (%u failed checks of %u)\n",
           failures ? "FAIL" : "PASS",
           failures,
           (unsigned)(ARRAY_COUNT(patterns) + 1u));
    printf("==================================================\n");
    fflush(stdout);

    while (true) sleep_ms(1000);
}
