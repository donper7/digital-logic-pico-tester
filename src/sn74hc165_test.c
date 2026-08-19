#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define STEP_US 5u

/*
 * SN74HC165N automatic functional tester
 * 8-bit parallel-in, serial-out shift register.
 *
 * Power:
 *   IC pin 16 -> Pico 3V3(OUT), physical pin 36
 *   IC pin 8  -> Pico GND, physical pin 38
 *
 * Pico outputs to DUT:
 *   GP2  (physical 4)  -> SH/LD,   IC pin 1 (active LOW load)
 *   GP3  (physical 5)  -> CLK,     IC pin 2
 *   GP4  (physical 6)  -> CLK INH, IC pin 15
 *   GP5  (physical 7)  -> SER,     IC pin 10
 *   GP6  (physical 9)  -> A,       IC pin 11
 *   GP7  (physical 10) -> B,       IC pin 12
 *   GP8  (physical 11) -> C,       IC pin 13
 *   GP9  (physical 12) -> D,       IC pin 14
 *   GP10 (physical 14) -> E,       IC pin 3
 *   GP11 (physical 15) -> F,       IC pin 4
 *   GP12 (physical 16) -> G,       IC pin 5
 *   GP13 (physical 17) -> H,       IC pin 6
 *
 * DUT outputs to Pico:
 *   QH       IC pin 9 -> GP14, physical 19
 *   QH-bar   IC pin 7 -> GP15, physical 20
 */

static const uint SHLD_GPIO = 2;
static const uint CLK_GPIO = 3;
static const uint CLK_INH_GPIO = 4;
static const uint SER_GPIO = 5;
static const uint parallel_gpios[8] = {6, 7, 8, 9, 10, 11, 12, 13};
static const uint QH_GPIO = 14;
static const uint QHBAR_GPIO = 15;

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
    init_output(SHLD_GPIO, 1);
    init_output(CLK_GPIO, 0);
    init_output(CLK_INH_GPIO, 0);
    init_output(SER_GPIO, 0);

    for (unsigned i = 0; i < 8; ++i) init_output(parallel_gpios[i], 0);

    gpio_init(QH_GPIO);
    gpio_set_dir(QH_GPIO, GPIO_IN);
    gpio_disable_pulls(QH_GPIO);

    gpio_init(QHBAR_GPIO);
    gpio_set_dir(QHBAR_GPIO, GPIO_IN);
    gpio_disable_pulls(QHBAR_GPIO);
}

static void set_parallel(uint8_t pattern)
{
    for (unsigned i = 0; i < 8; ++i) {
        gpio_put(parallel_gpios[i], (pattern >> i) & 1u);
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

static void parallel_load(uint8_t pattern)
{
    set_parallel(pattern);
    gpio_put(SHLD_GPIO, 0);
    sleep_us(STEP_US);
    gpio_put(SHLD_GPIO, 1);
    sleep_us(STEP_US);
}

static uint8_t read_shifted_byte(unsigned *complement_errors)
{
    uint8_t value = 0;
    unsigned errors = 0;

    for (int bit = 7; bit >= 0; --bit) {
        bool qh = gpio_get(QH_GPIO) != 0;
        bool qhbar = gpio_get(QHBAR_GPIO) != 0;

        if (qh) value |= (uint8_t)(1u << bit);
        if (qhbar == qh) ++errors;

        if (bit > 0) pulse_clock();
    }

    if (complement_errors) *complement_errors = errors;
    return value;
}

int main(void)
{
    stdio_init_all();
    init_fixture();
    wait_for_usb();

    printf("\n==================================================\n");
    printf("SN74HC165N AUTOMATIC FUNCTIONAL TEST\n");
    printf("8-bit parallel-in, serial-out shift register\n");
    printf("==================================================\n");

    unsigned failures = 0;
    const uint8_t patterns[] = {0x00, 0xFF, 0xA5, 0x5A, 0x81, 0x18};

    printf("\nLoaded | Shifted out | QH/QH-bar | Result\n");
    printf("-------+-------------+-----------+-------\n");

    for (size_t i = 0; i < ARRAY_COUNT(patterns); ++i) {
        gpio_put(CLK_INH_GPIO, 0);
        gpio_put(SER_GPIO, 0);
        parallel_load(patterns[i]);

        unsigned complement_errors = 0;
        uint8_t measured = read_shifted_byte(&complement_errors);
        bool pass = measured == patterns[i] && complement_errors == 0;

        printf(" 0x%02X  |    0x%02X     |    %s    | %s\n",
               patterns[i], measured,
               complement_errors ? "FAIL" : "PASS",
               pass ? "PASS" : "FAIL");
        if (!pass) ++failures;
    }

    /* Clock-inhibit check: load H=1 and G=0, then verify a CLK pulse is blocked. */
    gpio_put(SHLD_GPIO, 0);
    gpio_put(CLK_INH_GPIO, 1);
    set_parallel(0x80);
    sleep_us(STEP_US);
    gpio_put(SHLD_GPIO, 1);
    sleep_us(STEP_US);

    bool before = gpio_get(QH_GPIO) != 0;
    pulse_clock();
    bool after = gpio_get(QH_GPIO) != 0;
    bool inhibit_pass = before && after;
    printf("\nCLK INH test: QH before=%u after=%u %s\n",
           before, after, inhibit_pass ? "PASS" : "FAIL");
    if (!inhibit_pass) ++failures;

    /* Return CLK INH low while SH/LD is low so no unintended shift occurs. */
    gpio_put(SHLD_GPIO, 0);
    gpio_put(CLK_INH_GPIO, 0);
    set_parallel(0x00);
    sleep_us(STEP_US);
    gpio_put(SHLD_GPIO, 1);
    sleep_us(STEP_US);

    /* Serial-input check: eight 1 bits should propagate to QH after eight clocks. */
    gpio_put(SER_GPIO, 1);
    for (unsigned i = 0; i < 8; ++i) pulse_clock();
    bool serial_qh = gpio_get(QH_GPIO) != 0;
    bool serial_qhbar = gpio_get(QHBAR_GPIO) != 0;
    bool serial_pass = serial_qh && !serial_qhbar;
    printf("SER test: QH=%u QH-bar=%u %s\n",
           serial_qh, serial_qhbar, serial_pass ? "PASS" : "FAIL");
    if (!serial_pass) ++failures;

    gpio_put(SER_GPIO, 0);

    printf("\n==================================================\n");
    printf("FINAL RESULT: %s (%u failed checks of %u)\n",
           failures ? "FAIL" : "PASS",
           failures,
           (unsigned)(ARRAY_COUNT(patterns) + 2u));
    printf("==================================================\n");
    fflush(stdout);

    while (true) sleep_ms(1000);
}
