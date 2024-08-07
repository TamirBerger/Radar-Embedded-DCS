#include  "../header/api.h"    		// private library - API layer
#include  "../header/halGPIO.h"     // private library - HAL layer


// LCD funcs
//******************************************************************
// write a string of chars to the LCD
//******************************************************************
void lcd_puts(const char * s){
    int i=0;
  
	while(*s){
	    if (i == 16)
	        lcd_new_line;
		lcd_data(*s++);
		i++;
	}
}

void print_number(int number){
    int y;
    int n;
    int count;
    int j;

    n = number;
    count = 0;
    while (n != 0){
        n /= 10;
        count++;
    }

    for (j = count; j > 0 ; j--)
        lcd_cursor_right();

    while (number >= 10){
        lcd_cursor_left();
        y = number % 10;
        lcd_data(y + '0'); // convert a digit to the corresponding character
        number /= 10;
        lcd_cursor_left();
    }

    lcd_cursor_left();
    lcd_data(number + '0');
}


void counterup(int counter){
    lcd_clear();
    lcd_home();
    print_number(counter);
}


void counterdown(int counter){
    lcd_clear();
    lcd_home();
    print_number(counter);
}


//-------------------------------------------------------------------------------------
//           store integer value in a string
//-------------------------------------------------------------------------------------
void intToDecStr(int value, char* str) {
    int hundreds = 0, tens = 0, ones = 0;

    while (value >= 100) {
        value -= 100;
        hundreds += 1;
    }
    while (value >= 10) {
        value -= 10;
        tens += 1;
    }
    while (value >= 1) {
        value -= 1;
        ones += 1;
    }
    str[0] = '0' + hundreds;
    str[1] = '0' + tens;
    str[2] = '0' + ones;
    str[3] = '\0';
}
