#ifndef _halGPIO_H_
#define _halGPIO_H_

#include "../header/bsp_msp430x4xx.h"

#include  "../header/app.h"    		// private library - APP layer


//******************************************************************
//                   global variable
//******************************************************************
extern enum FSMstate state;  
extern enum SYSmode lpm_mode; 


//******************************************************************
//                     funcs
//******************************************************************
extern void sysConfig(void);
extern void delay(unsigned int);
extern void enterLPM(unsigned char);
extern void enable_interrupts();
extern void disable_interrupts();
extern void delay_func(int delay);
extern void transmit_dist_arr();
extern void transmit_num();
extern void transmit_str(char* str);
extern void recieve_X_ms();
extern int get_X_ms();
extern void LDR_value();
extern void pwm_calc();
extern void pwm_servo(int angle);
extern void pwm_distance();
extern void capture();

// state2
extern int update_flag_user_angle();
extern unsigned int get_user_angle();
extern void send_distance();

// state5
extern void set_flash_ptr();
extern int get_write_flash_flag();
extern void write_Seg ();
extern void erase_seg();
extern int AsciToint1(char str1, char str2);
extern void script(int scirpt_number);
extern void DelayMs_loop(unsigned int cnt);
extern void transmit_ldr_arr();

//******************************************************************
//                     interrupt
//******************************************************************
extern __interrupt void Timer0(void);
extern __interrupt void Timer1(void);

extern __interrupt void USCI0TX_ISR(void);
extern __interrupt void USCI0RX_ISR(void);

//******************************************************************
//                      LCD 
//******************************************************************
extern void lcd_cmd(unsigned char);
extern void lcd_data(unsigned char);
extern void lcd_init();
extern void lcd_strobe();
extern void DelayMs(unsigned int);
extern void DelayUs(unsigned int);

//******************************************************************
//                      Uart
//******************************************************************
extern void uart_send_array(int size);

#endif
