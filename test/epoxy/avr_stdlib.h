/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

/**
 * @file avr_stdlib.h
 *
 * Non-standard functions from AVR's <stdlib.h> library which do not exist on
 * Linux or MacOS. This contains only the ones required to compile
 * AUnit tests on Linux or MacOS.
 */

#ifndef EPOXY_DUINO_AVR_STDLIB_H
#define EPOXY_DUINO_AVR_STDLIB_H

extern "C" {

char *itoa(int n, char *str, int base);
char *utoa(unsigned n, char *str, int base);
char *ltoa(long n, char *str, int base);
char *ultoa(unsigned long n, char *str, int base);
char *dtostrf(double val, signed char width, unsigned char prec, char *s);

}

#endif
