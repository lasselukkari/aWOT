/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

/**
 * @file pgmspace.h
 *
 * A version of the <avr/pgmspace.h> file in the Arduino environment with
 * enough mappings to allow AUnit tests to compile and run on Linux or MacOS.
 */

#ifndef EPOXY_DUINO_PGMSPACE_H
#define EPOXY_DUINO_PGMSPACE_H

#include <string.h>

#define PGM_P const char *
#define PGM_VOID_P const void *
#define PSTR(str) (str)

#define PROGMEM

#define pgm_read_byte(p) (* (const uint8_t*) (p))
#define pgm_read_word(p) (* (const uint16_t*) (p))
#define pgm_read_dword(p) (* (const uint32_t*) (p))
#define pgm_read_float(p) (* (const float*) (p))
#define pgm_read_ptr(p) (* (const void* const*) (p))

#define pgm_read_byte_near(addr)  pgm_read_byte(addr)
#define pgm_read_word_near(addr)  pgm_read_word(addr)
#define pgm_read_dword_near(addr) pgm_read_dword(addr)
#define pgm_read_float_near(addr) pgm_read_float(addr)
#define pgm_read_ptr_near(addr)   pgm_read_ptr(addr)

#define pgm_read_byte_far(addr)   pgm_read_byte(addr)
#define pgm_read_word_far(addr)   pgm_read_word(addr)
#define pgm_read_dword_far(addr)  pgm_read_dword(addr)
#define pgm_read_float_far(addr)  pgm_read_float(addr)
#define pgm_read_ptr_far(addr)    pgm_read_ptr(addr)

#define memcmp_P memcmp
#define memcpy_P memcpy
#define snprintf_P snprintf
#define strcasecmp_P strcasecmp
#define strcat_P strcat
#define strchr_P strchr
#define strcmp_P strcmp
#define strcpy_P strcpy
#define strlen_P strlen
#define strncasecmp_P strncasecmp
#define strncat_P strncat
#define strncmp_P strncmp
#define strncpy_P strncpy
#define strrchr_P strrchr
#define strstr_P strstr
#define vsnprintf_P vsnprintf

#endif
