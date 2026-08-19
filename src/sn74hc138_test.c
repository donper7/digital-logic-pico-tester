#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#define SETTLE_US 10u

/*
 * SN74HC138N automatic functional tester
 * 3-to-8 decoder/demultiplexer with active-LOW outputs.
 *
 * Power:
 *   IC pin 16 -> Pico 3V3(OUT), physical pin 36
 *   IC pin 8  -> Pico GND, physical pin 38
 *
 * Pico outputs to DUT:
 *   GP2  (physical 4)  -> A,     IC pin 1
 *   GP3  (physical 5)  -> B,     IC pin 2
 *   GP4  (physical 6)  -> C,     IC pin 3
 *   GP5  (physical 7)  -> G2A,   IC pin 4 (active LOW)
 *   GP6  (physical 9)  -> G2B,   IC pin 5 (active LOW)
 *   GP7  (physical 10) -> G1,    IC pin 6 (active HIGH)
 *
 * DUT outputs to Pico:
 *   IC pin 15 Y0 -> GP8,  physical 11
 *   IC pin 14 Y1 -> GP9,  physical 12
 *   IC pin 13 Y2 -> GP10, physical 14
 *   IC pin 12 Y3 -> GP11, physical 15
 *   IC pin 11 Y4 -> GP12, physical 16
 *   IC pin 10 Y5 -> GP13, physical 17
 *   IC pin 9  Y6 -> GP14, physical 19
 *   IC pin 7  Y7 -> GP15, physical 20
 */

static const uint A_GPIO = 2;
static const uint B_GPIO = 3;
static const uint C_GPIO = 4;
static const uint G2A_GPIO = 5;
static const uint G2B_GPIO = 6;
static const uint G1_GPIO = 7;
static const uint y_gpios[8] = {8, 9, 10, 11, 12, 13, 14, 15};

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
    init_output(C_GPIO, 0);
    init_output(G2A_GPIO, 0);
    init_output(G2B_GPIO, 0);
    init_output(G1_GPIO, 0);

    for (unsigned i = 0; i < 8; ++i) {
        gpio_init(y_gpios[i]);
        gpio_set_dir(y_gpios[i], GPIO_IN);
        gpio_disable_pulls(y_gpios[i]);
    }
}

static void set_address(unsigned address)
{
    gpio_put(A_GPIO, (address >> 0u) & 1u);
    gpio_put(B_GPIO, (address >> 1u) & 1u);
    gpio_put(C_GPIO, (address >> 2u) & 1u);
}

static void set_enable(bool g1, bool g2a, bool g2b)
{
    gpio_put(G1_GPIO, g1);
    gpio_put(G2A_GPIO, g2a);
    gpio_put(G2B_GPIO, g2b);
}

static uint8_t read_outputs(void)
{
    uint8_t value = 0;
    for (unsigned i = 0; i < 8; ++i) {
        if (gpio_get(y_gpios[i])) value |= (uint8_t)(1u << i);
    }
    return value;
}

static void print_binary(uint8_t value)
{
    for (int bit = 7; bit >= 0; --bit) {
        putchar((value & (1u << bit)) ? '1' : '0');
    }
}

int main(void)
{
    stdio_init_all();
    init_fixture();
    wait_for_usb();

    printf("\n============================================================\n");
    printf("SN74HC138N AUTOMATIC FUNCTIONAL TEST\n");
    printf("3-to-8 decoder; outputs Y0-Y7 are active LOW\n");
    printf("============================================================\n");

    unsigned failures = 0;

    set_enable(1, 0, 0);
    printf("\nEnabled decoder tests\n");
    printf("Addr CBA | Selected | Expected Y7..Y0 | Measured Y7..Y0 | Result\n");
    printf("---------+----------+-----------------+-----------------+-------\n");

    for (unsigned address = 0; address < 8; ++address) {
        set_address(address);
        sleep_us(SETTLE_US);

        uint8_t expected = (uint8_t)(0xFFu & ~(1u << address));
        uint8_t measured = read_outputs();
        bool pass = measured == expected;

        printf(" %u%u%u     |   Y%u     | ",
               (address >> 2u) & 1u,
               (address >> 1u) & 1u,
               address & 1u,
               address);
        print_binary(expected);
        printf("        | ");
        print_binary(measured);
        printf("        | %s\n", pass ? "PASS" : "FAIL");

        if (!pass) ++failures;
    }

    printf("\nEnable-input tests (all outputs should be HIGH)\n");
    printf(" G1 G2A G2B | Measured Y7..Y0 | Result\n");
    printf("------------+-----------------+-------\n");

    const bool enable_tests[3][3] = {
        {0, 0, 0},
        {1, 1, 0},
        {1, 0, 1}
    };

    set_address(0);
    for (unsigned i = 0; i < 3; ++i) {
        set_enable(enable_tests[i][0], enable_tests[i][1], enable_tests[i][2]);
        sleep_us(SETTLE_US);

        uint8_t measured = read_outputs();
        bool pass = measured == 0xFFu;

        printf("  %u   %u   %u  | ",
               enable_tests[i][0], enable_tests[i][1], enable_tests[i][2]);
        print_binary(measured);
        printf("        | %s\n", pass ? "PASS" : "FAIL");

        if (!pass) ++failures;
    }

    set_enable(0, 0, 0);

    printf("\n============================================================\n");
    printf("FINAL RESULT: %s (%u failed checks of 11)\n",
           failures ? "FAIL" : "PASS", failures);
    printf("============================================================\n");
    fflush(stdout);

    while (true) sleep_ms(1000);
}
