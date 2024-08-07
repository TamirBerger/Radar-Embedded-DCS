#ifndef _api_H_
#define _api_H_

#include  "../header/halGPIO.h"     // private library - HAL layer

// high level functions
extern void intToDecStr(int value, char* str);

// LCD
extern void lcd_puts(const char * s);

extern void print_number(int number);
extern void counterdown(int counter);
extern void counterup(int counter);

#endif
