#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define SETTLE_US 10u

/*
 * SN74HC02N automatic functional tester
 * Quad 2-input NOR gate
 *
 * Power:
 *   VCC -> Pico 3V3(OUT), physical pin 36
 *   GND -> Pico GND, physical pin 38
 *
 * Pico mapping:
 *   GP2  (physical 4)  -> IC pin 2
 *   GP3  (physical 5)  -> IC pin 3
 *   GP4  (physical 6)  -> IC pin 5
 *   GP5  (physical 7)  -> IC pin 6
 *   GP6  (physical 9)  -> IC pin 8
 *   GP7  (physical 10) -> IC pin 9
 *   GP8  (physical 11) -> IC pin 11
 *   GP9  (physical 12) -> IC pin 12
 *   GP10 (physical 14) <- IC pin 1
 *   GP11 (physical 15) <- IC pin 4
 *   GP12 (physical 16) <- IC pin 10
 *   GP13 (physical 17) <- IC pin 13
 */

typedef struct {
    const char *name;
    uint a_gpio, b_gpio, y_gpio;
    uint8_t a_pin, b_pin, y_pin;
} gate_t;

static const gate_t gates[] = {
    {"Gate 1", 2, 3, 10, 2, 3, 1},
    {"Gate 2", 4, 5, 11, 5, 6, 4},
    {"Gate 3", 6, 7, 12, 8, 9, 10},
    {"Gate 4", 8, 9, 13, 11, 12, 13},
};

static bool expected_output(bool a, bool b)
{
    return !(a || b);
}

static void wait_for_usb(void)
{
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(250);
}

static void init_fixture(void)
{
    for (size_t i = 0; i < ARRAY_COUNT(gates); ++i) {
        gpio_init(gates[i].a_gpio);
        gpio_put(gates[i].a_gpio, 0);
        gpio_set_dir(gates[i].a_gpio, GPIO_OUT);

        gpio_init(gates[i].b_gpio);
        gpio_put(gates[i].b_gpio, 0);
        gpio_set_dir(gates[i].b_gpio, GPIO_OUT);

        gpio_init(gates[i].y_gpio);
        gpio_set_dir(gates[i].y_gpio, GPIO_IN);
        gpio_disable_pulls(gates[i].y_gpio);
    }
}

static unsigned test_gate(const gate_t *g)
{
    unsigned failures = 0;

    printf("\n%s: A=IC pin %u, B=IC pin %u, Y=IC pin %u\n",
           g->name, g->a_pin, g->b_pin, g->y_pin);
    printf(" A  B | Expected  Measured | Result\n");
    printf("------+--------------------+--------\n");

    for (unsigned v = 0; v < 4; ++v) {
        bool a = ((v >> 1u) & 1u) != 0;
        bool b = (v & 1u) != 0;

        gpio_put(g->a_gpio, a);
        gpio_put(g->b_gpio, b);
        sleep_us(SETTLE_US);

        bool expected = expected_output(a, b);
        bool measured = gpio_get(g->y_gpio) != 0;
        bool pass = measured == expected;

        printf(" %u  %u |    %u          %u    | %s\n",
               a, b, expected, measured, pass ? "PASS" : "FAIL");

        if (!pass) ++failures;
    }

    gpio_put(g->a_gpio, 0);
    gpio_put(g->b_gpio, 0);
    printf("%s result: %s\n", g->name, failures ? "FAIL" : "PASS");
    return failures;
}

int main(void)
{
    stdio_init_all();
    init_fixture();
    wait_for_usb();

    printf("\n==================================================\n");
    printf("SN74HC02N AUTOMATIC FUNCTIONAL TEST\n");
    printf("Quad 2-input NOR gate\n");
    printf("==================================================\n");

    unsigned failed_gates = 0;
    unsigned failed_cases = 0;

    for (size_t i = 0; i < ARRAY_COUNT(gates); ++i) {
        unsigned f = test_gate(&gates[i]);
        failed_cases += f;
        if (f) ++failed_gates;
    }

    printf("\n==================================================\n");
    printf("FINAL RESULT\n");
    printf("==================================================\n");
    printf("Gates passed:       %u of 4\n", 4u - failed_gates);
    printf("Test cases passed:  %u of 16\n", 16u - failed_cases);
    printf("Device result:      %s\n", failed_cases ? "FAIL" : "PASS");
    printf("==================================================\n");
    fflush(stdout);

    while (true) sleep_ms(1000);
}
