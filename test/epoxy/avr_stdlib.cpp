/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "avr_stdlib.h"

#define BUFSIZE (sizeof(int) * 8 + 1)

// Copied and modified from https://people.cs.umu.se/isak/snippets/ltoa.c
// Copyright 1988-90 by Robert B. Stout dba MicroFirm
// Released to public domain, 1991.
char *itoa(int n, char *str, int base) {
  if (36 < base || 2 > base) {
    base = 10; /* can only use 0-9, A-Z        */
  }

  char buf[BUFSIZE];
  char *tail = &buf[BUFSIZE - 1]; /* last character position      */
  *tail-- = '\0';

  char *head = str;
  unsigned uarg;
  if (10 == base && n < 0) {
    *head++ = '-';
    uarg = -n;
  } else {
    uarg = n;
  }

  unsigned i = 2;
  if (uarg) {
    for (i = 1; uarg; ++i) {
      div_t r = div(uarg, base);
      *tail-- = (char)(r.rem + ((9 < r.rem) ? ('A' - 10) : '0'));
      uarg = r.quot;
    }
  } else {
    *tail-- = '0';
  }

  memcpy(head, ++tail, i);
  return str;
}

// Copied and modified from https://people.cs.umu.se/isak/snippets/ltoa.c
// Copyright 1988-90 by Robert B. Stout dba MicroFirm
// Released to public domain, 1991.
char *utoa(unsigned n, char *str, int base) {
  if (36 < base || 2 > base) {
    base = 10; /* can only use 0-9, A-Z        */
  }

  char buf[BUFSIZE];
  char *tail = &buf[BUFSIZE - 1]; /* last character position      */
  *tail-- = '\0';

  unsigned i = 2;
  if (n) {
    for (i = 1; n; ++i) {
      div_t r = div(n, base);
      *tail-- = (char)(r.rem + ((9 < r.rem) ? ('A' - 10) : '0'));
      n = r.quot;
    }
  } else {
    *tail-- = '0';
  }

  memcpy(str, ++tail, i);
  return str;
}

// Copied and modified from https://people.cs.umu.se/isak/snippets/ltoa.c
// Copyright 1988-90 by Robert B. Stout dba MicroFirm
// Released to public domain, 1991.
char *ltoa(long n, char *str, int base) {
  if (36 < base || 2 > base) {
    base = 10; /* can only use 0-9, A-Z        */
  }

  char buf[BUFSIZE];
  char *tail = &buf[BUFSIZE - 1]; /* last character position      */
  *tail-- = '\0';

  char *head = str;
  unsigned long uarg;
  if (10 == base && n < 0) {
    *head++ = '-';
    uarg = -n;
  } else {
    uarg = n;
  }

  unsigned i = 2;
  if (uarg) {
    for (i = 1; uarg; ++i) {
      ldiv_t r = ldiv(uarg, base);
      *tail-- = (char)(r.rem + ((9 < r.rem) ? ('A' - 10) : '0'));
      uarg = r.quot;
    }
  } else {
    *tail-- = '0';
  }

  memcpy(head, ++tail, i);
  return str;
}

// Copied and modified from https://people.cs.umu.se/isak/snippets/ltoa.c
// Copyright 1988-90 by Robert B. Stout dba MicroFirm
// Released to public domain, 1991.
char *ultoa(unsigned long n, char *str, int base) {
  if (36 < base || 2 > base) {
    base = 10; /* can only use 0-9, A-Z        */
  }

  char buf[BUFSIZE];
  char *tail = &buf[BUFSIZE - 1]; /* last character position      */
  *tail-- = '\0';

  unsigned i = 2;
  if (n) {
    for (i = 1; n; ++i) {
      ldiv_t r = ldiv(n, base);
      *tail-- = (char)(r.rem + ((9 < r.rem) ? ('A' - 10) : '0'));
      n = r.quot;
    }
  } else {
    *tail-- = '0';
  }

  memcpy(str, ++tail, i);
  return str;
}

// This is a terrible, hacky implementation of dtostrf() but this will never be
// used in production. It is only used to allow Arduino unit tests using AUnit
// to compile under Linux and MacOS.
char *dtostrf(double val, signed char width, unsigned char prec, char *s) {
  char format[13];
  char swidth[5];
  itoa(width, swidth, 10);
  char sprec[5];
  utoa(prec, sprec, 10);

  strcpy(format, "%");
  strcat(format, swidth);
  strcat(format, ".");
  strcat(format, sprec);
  strcat(format, "f");

  sprintf(s, format, val);
  return s;
}
