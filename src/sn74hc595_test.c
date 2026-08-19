#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define STEP_US 5u

/*
 * SN74HC595N automatic functional tester
 * 8-bit serial-in, parallel-out shift register with storage register.
 *
 * Power:
 *   IC pin 16 -> Pico 3V3(OUT), physical pin 36
 *   IC pin 8  -> Pico GND, physical pin 38
 *
 * Pico outputs to DUT:
 *   GP2 (physical 4) -> SER,    IC pin 14
 *   GP3 (physical 5) -> SRCLK,  IC pin 11
 *   GP4 (physical 6) -> RCLK,   IC pin 12
 *   GP5 (physical 7) -> SRCLR,  IC pin 10 (active LOW)
 *   GP6 (physical 9) -> OE,     IC pin 13 (active LOW)
 *
 * DUT outputs to Pico:
 *   QA pin 15 -> GP7,  physical 10
 *   QB pin 1  -> GP8,  physical 11
 *   QC pin 2  -> GP9,  physical 12
 *   QD pin 3  -> GP10, physical 14
 *   QE pin 4  -> GP11, physical 15
 *   QF pin 5  -> GP12, physical 16
 *   QG pin 6  -> GP13, physical 17
 *   QH pin 7  -> GP14, physical 19
 *   QH' pin 9 -> GP15, physical 20
 */

static const uint SER_GPIO = 2;
static const uint SRCLK_GPIO = 3;
static const uint RCLK_GPIO = 4;
static const uint SRCLR_GPIO = 5;
static const uint OE_GPIO = 6;
static const uint q_gpios[8] = {7, 8, 9, 10, 11, 12, 13, 14};
static const uint QH_PRIME_GPIO = 15;

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
    init_output(SER_GPIO, 0);
    init_output(SRCLK_GPIO, 0);
    init_output(RCLK_GPIO, 0);
    init_output(SRCLR_GPIO, 1);
    init_output(OE_GPIO, 0);

    for (unsigned i = 0; i < 8; ++i) {
        gpio_init(q_gpios[i]);
        gpio_set_dir(q_gpios[i], GPIO_IN);
        gpio_disable_pulls(q_gpios[i]);
    }

    gpio_init(QH_PRIME_GPIO);
    gpio_set_dir(QH_PRIME_GPIO, GPIO_IN);
    gpio_disable_pulls(QH_PRIME_GPIO);
}

static void pulse(uint gpio)
{
    gpio_put(gpio, 0);
    sleep_us(STEP_US);
    gpio_put(gpio, 1);
    sleep_us(STEP_US);
    gpio_put(gpio, 0);
    sleep_us(STEP_US);
}

static void clear_shift_register(void)
{
    gpio_put(SRCLR_GPIO, 0);
    sleep_us(STEP_US);
    gpio_put(SRCLR_GPIO, 1);
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

static bool load_pattern(uint8_t pattern)
{
    clear_shift_register();

    for (int bit = 7; bit >= 0; --bit) {
        gpio_put(SER_GPIO, (pattern >> bit) & 1u);
        pulse(SRCLK_GPIO);
    }

    bool qh_prime = gpio_get(QH_PRIME_GPIO) != 0;
    bool qh_prime_ok = qh_prime == (((pattern >> 7u) & 1u) != 0);

    pulse(RCLK_GPIO);
    gpio_put(SER_GPIO, 0);
    return qh_prime_ok;
}

int main(void)
{
    stdio_init_all();
    init_fixture();
    wait_for_usb();

    printf("\n==================================================\n");
    printf("SN74HC595N AUTOMATIC FUNCTIONAL TEST\n");
    printf("8-bit serial-in, parallel-out shift/storage register\n");
    printf("==================================================\n");

    unsigned failures = 0;

    clear_shift_register();
    pulse(RCLK_GPIO);
    uint8_t measured = read_parallel();
    bool clear_pass = measured == 0x00u;
    printf("\nSRCLR + RCLK test: expected=0x00 measured=0x%02X %s\n",
           measured, clear_pass ? "PASS" : "FAIL");
    if (!clear_pass) ++failures;

    const uint8_t patterns[] = {0x00, 0xFF, 0xA5, 0x5A, 0x81, 0x18};

    printf("\nPattern | Parallel out | QH' | Result\n");
    printf("--------+--------------+-----+-------\n");

    for (size_t i = 0; i < ARRAY_COUNT(patterns); ++i) {
        bool cascade_ok = load_pattern(patterns[i]);
        measured = read_parallel();
        bool pass = measured == patterns[i] && cascade_ok;

        printf(" 0x%02X   |     0x%02X     | %s | %s\n",
               patterns[i], measured,
               cascade_ok ? "PASS" : "FAIL",
               pass ? "PASS" : "FAIL");
        if (!pass) ++failures;
    }

    /* OE test: latch all HIGH, disable outputs, and use Pico pulldowns to verify high-Z. */
    load_pattern(0xFF);
    for (unsigned i = 0; i < 8; ++i) gpio_pull_down(q_gpios[i]);
    gpio_put(OE_GPIO, 1);
    sleep_us(50);
    uint8_t disabled_read = read_parallel();
    bool oe_pass = disabled_read == 0x00u;
    printf("\nOE high (outputs high-Z) test: measured with pulldowns=0x%02X %s\n",
           disabled_read, oe_pass ? "PASS" : "FAIL");
    if (!oe_pass) ++failures;

    gpio_put(OE_GPIO, 0);
    for (unsigned i = 0; i < 8; ++i) gpio_disable_pulls(q_gpios[i]);

    clear_shift_register();
    pulse(RCLK_GPIO);

    printf("\n==================================================\n");
    printf("FINAL RESULT: %s (%u failed checks of %u)\n",
           failures ? "FAIL" : "PASS",
           failures,
           (unsigned)(ARRAY_COUNT(patterns) + 2u));
    printf("==================================================\n");
    fflush(stdout);

    while (true) sleep_ms(1000);
}
