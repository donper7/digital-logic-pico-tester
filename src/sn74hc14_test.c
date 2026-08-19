#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define SETTLE_US 10u

/*
 * SN74HC14N automatic functional tester
 * Hex Schmitt-trigger inverter
 * This is a static logic-function test; it does not measure Schmitt hysteresis thresholds.
 *
 * Power:
 *   IC pin 14 -> Pico 3V3(OUT), physical pin 36
 *   IC pin 7  -> Pico GND, physical pin 38
 *
 * Pico mapping:
 *   GP2  (physical 4)  -> IC pin 1;  GP8  (physical 11) <- IC pin 2
 *   GP3  (physical 5)  -> IC pin 3;  GP9  (physical 12) <- IC pin 4
 *   GP4  (physical 6)  -> IC pin 5;  GP10 (physical 14) <- IC pin 6
 *   GP5  (physical 7)  -> IC pin 9;  GP11 (physical 15) <- IC pin 8
 *   GP6  (physical 9)  -> IC pin 11; GP12 (physical 16) <- IC pin 10
 *   GP7  (physical 10) -> IC pin 13; GP13 (physical 17) <- IC pin 12
 */

typedef struct {
    const char *name;
    uint in_gpio, out_gpio;
    uint8_t in_pin, out_pin;
} inverter_t;

static const inverter_t channels[] = {
    {"Inverter 1", 2, 8, 1, 2},
    {"Inverter 2", 3, 9, 3, 4},
    {"Inverter 3", 4, 10, 5, 6},
    {"Inverter 4", 5, 11, 9, 8},
    {"Inverter 5", 6, 12, 11, 10},
    {"Inverter 6", 7, 13, 13, 12},
};

static void wait_for_usb(void)
{
    while (!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(250);
}

static void init_fixture(void)
{
    for (size_t i = 0; i < ARRAY_COUNT(channels); ++i) {
        gpio_init(channels[i].in_gpio);
        gpio_put(channels[i].in_gpio, 0);
        gpio_set_dir(channels[i].in_gpio, GPIO_OUT);

        gpio_init(channels[i].out_gpio);
        gpio_set_dir(channels[i].out_gpio, GPIO_IN);
        gpio_disable_pulls(channels[i].out_gpio);
    }
}

static unsigned test_channel(const inverter_t *c)
{
    unsigned failures = 0;

    printf("\n%s: input=IC pin %u, output=IC pin %u\n",
           c->name, c->in_pin, c->out_pin);
    printf(" In | Expected  Measured | Result\n");
    printf("----+--------------------+--------\n");

    for (unsigned input = 0; input < 2; ++input) {
        gpio_put(c->in_gpio, input);
        sleep_us(SETTLE_US);

        bool expected = !input;
        bool measured = gpio_get(c->out_gpio) != 0;
        bool pass = measured == expected;

        printf("  %u |    %u          %u    | %s\n",
               input, expected, measured, pass ? "PASS" : "FAIL");
        if (!pass) ++failures;
    }

    gpio_put(c->in_gpio, 0);
    printf("%s result: %s\n", c->name, failures ? "FAIL" : "PASS");
    return failures;
}

int main(void)
{
    stdio_init_all();
    init_fixture();
    wait_for_usb();

    printf("\n==================================================\n");
    printf("SN74HC14N AUTOMATIC FUNCTIONAL TEST\n");
    printf("Hex Schmitt-trigger inverter\n");
    printf("==================================================\n");

    unsigned failed_channels = 0;
    unsigned failed_cases = 0;

    for (size_t i = 0; i < ARRAY_COUNT(channels); ++i) {
        unsigned f = test_channel(&channels[i]);
        failed_cases += f;
        if (f) ++failed_channels;
    }

    printf("\n==================================================\n");
    printf("FINAL RESULT\n");
    printf("==================================================\n");
    printf("Channels passed:    %u of 6\n", 6u - failed_channels);
    printf("Test cases passed:  %u of 12\n", 12u - failed_cases);
    printf("Device result:      %s\n", failed_cases ? "FAIL" : "PASS");
    printf("==================================================\n");
    fflush(stdout);

    while (true) sleep_ms(1000);
}
