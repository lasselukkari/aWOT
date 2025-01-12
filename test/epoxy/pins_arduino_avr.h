/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#ifndef EPOXY_DUINO_PINS_ARDUINO_AVR_H
#define EPOXY_DUINO_PINS_ARDUINO_AVR_H

// Arbitrarily define the pin for the LED_BUILTIN
#define LED_BUILTIN 1

// Define a bunch of digital pins.
#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define D8 8
#define D9 9
#define D10 10
#define D11 11
#define D12 12
#define D13 13
#define D14 14
#define D15 15
#define D16 16
#define D17 17
#define D18 18
#define D19 19

// Define some arbitary analog pins.
#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 4
#define A5 5
#define A6 6
#define A7 7
#define A8 8
#define A9 9

// Define some arbitrary pins used by I2C and SPI.
static const uint8_t SS   = 1;
static const uint8_t MOSI = 2;
static const uint8_t MISO = 3;
static const uint8_t SCK  = 4;
static const uint8_t SDA = 5;
static const uint8_t SCL = 6;

#endif
