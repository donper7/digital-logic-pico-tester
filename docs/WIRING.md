# Wiring Reference

All mappings below list both the Raspberry Pi Pico GPIO number and the Pico physical header pin.

## Common power

For 14-pin devices:

- IC VCC pin 14 -> Pico 3V3(OUT), physical pin 36
- IC GND pin 7 -> Pico GND, physical pin 38

For 16-pin devices:

- IC VCC pin 16 -> Pico 3V3(OUT), physical pin 36
- IC GND pin 8 -> Pico GND, physical pin 38

Use a 0.1 uF ceramic bypass capacitor across the IC VCC and GND pins.

## SN74HC00N - NAND

- IC 1 <- GP2, physical 4
- IC 2 <- GP3, physical 5
- IC 3 -> GP10, physical 14
- IC 4 <- GP4, physical 6
- IC 5 <- GP5, physical 7
- IC 6 -> GP11, physical 15
- IC 8 -> GP12, physical 16
- IC 9 <- GP6, physical 9
- IC 10 <- GP7, physical 10
- IC 11 -> GP13, physical 17
- IC 12 <- GP8, physical 11
- IC 13 <- GP9, physical 12

## SN74HC02N - NOR

- IC 1 -> GP10, physical 14
- IC 2 <- GP2, physical 4
- IC 3 <- GP3, physical 5
- IC 4 -> GP11, physical 15
- IC 5 <- GP4, physical 6
- IC 6 <- GP5, physical 7
- IC 8 <- GP6, physical 9
- IC 9 <- GP7, physical 10
- IC 10 -> GP12, physical 16
- IC 11 <- GP8, physical 11
- IC 12 <- GP9, physical 12
- IC 13 -> GP13, physical 17

## SN74HC04N - Hex inverter

- IC 1 <- GP2, physical 4; IC 2 -> GP8, physical 11
- IC 3 <- GP3, physical 5; IC 4 -> GP9, physical 12
- IC 5 <- GP4, physical 6; IC 6 -> GP10, physical 14
- IC 9 <- GP5, physical 7; IC 8 -> GP11, physical 15
- IC 11 <- GP6, physical 9; IC 10 -> GP12, physical 16
- IC 13 <- GP7, physical 10; IC 12 -> GP13, physical 17

## SN74HC08N - AND

Same Pico mapping as SN74HC00N.

## SN74HC14N - Hex Schmitt-trigger inverter

Same Pico mapping as SN74HC04N.

## SN74HC32N - OR

Same Pico mapping as SN74HC00N.

## SN74HC138N - 3-to-8 decoder

Pico -> IC:

- GP2, physical 4 -> IC 1 (A)
- GP3, physical 5 -> IC 2 (B)
- GP4, physical 6 -> IC 3 (C)
- GP5, physical 7 -> IC 4 (G2A, active LOW)
- GP6, physical 9 -> IC 5 (G2B, active LOW)
- GP7, physical 10 -> IC 6 (G1, active HIGH)

IC -> Pico:

- IC 15 (Y0) -> GP8, physical 11
- IC 14 (Y1) -> GP9, physical 12
- IC 13 (Y2) -> GP10, physical 14
- IC 12 (Y3) -> GP11, physical 15
- IC 11 (Y4) -> GP12, physical 16
- IC 10 (Y5) -> GP13, physical 17
- IC 9 (Y6) -> GP14, physical 19
- IC 7 (Y7) -> GP15, physical 20

## SN74HC164N - serial-in, parallel-out

Pico -> IC:

- GP2, physical 4 -> IC 1 (A)
- GP3, physical 5 -> IC 2 (B)
- GP4, physical 6 -> IC 8 (CLK)
- GP5, physical 7 -> IC 9 (CLR, active LOW)

IC -> Pico:

- IC 3 (QA) -> GP6, physical 9
- IC 4 (QB) -> GP7, physical 10
- IC 5 (QC) -> GP8, physical 11
- IC 6 (QD) -> GP9, physical 12
- IC 10 (QE) -> GP10, physical 14
- IC 11 (QF) -> GP11, physical 15
- IC 12 (QG) -> GP12, physical 16
- IC 13 (QH) -> GP13, physical 17

## SN74HC165N - parallel-in, serial-out

Pico -> IC:

- GP2, physical 4 -> IC 1 (SH/LD, active LOW)
- GP3, physical 5 -> IC 2 (CLK)
- GP4, physical 6 -> IC 15 (CLK INH)
- GP5, physical 7 -> IC 10 (SER)
- GP6, physical 9 -> IC 11 (A)
- GP7, physical 10 -> IC 12 (B)
- GP8, physical 11 -> IC 13 (C)
- GP9, physical 12 -> IC 14 (D)
- GP10, physical 14 -> IC 3 (E)
- GP11, physical 15 -> IC 4 (F)
- GP12, physical 16 -> IC 5 (G)
- GP13, physical 17 -> IC 6 (H)

IC -> Pico:

- IC 9 (QH) -> GP14, physical 19
- IC 7 (QH-bar) -> GP15, physical 20

## SN74HC595N - serial-in, parallel-out with latch

Pico -> IC:

- GP2, physical 4 -> IC 14 (SER)
- GP3, physical 5 -> IC 11 (SRCLK)
- GP4, physical 6 -> IC 12 (RCLK)
- GP5, physical 7 -> IC 10 (SRCLR, active LOW)
- GP6, physical 9 -> IC 13 (OE, active LOW)

IC -> Pico:

- IC 15 (QA) -> GP7, physical 10
- IC 1 (QB) -> GP8, physical 11
- IC 2 (QC) -> GP9, physical 12
- IC 3 (QD) -> GP10, physical 14
- IC 4 (QE) -> GP11, physical 15
- IC 5 (QF) -> GP12, physical 16
- IC 6 (QG) -> GP13, physical 17
- IC 7 (QH) -> GP14, physical 19
- IC 9 (QH') -> GP15, physical 20
