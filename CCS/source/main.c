#include  "../header/api.h"    		// private library - API layer
#include  "../header/app.h"    		// private library - APP layer
#define AngleSpace 3

enum FSMstate state;
enum SYSmode lpm_mode;


void main(void){
    const char* str_delay = "enter X_ms";
    unsigned int X_ms = 500;
    unsigned int angle=0;              

    // state2
    int flag_user_angle;

    // state5
    int write_flash_flag = 0;
    int num_of_files = 0;
    char* adr = 0x1000;
    char scirpt_number;
    int script_number_int = 0;

    state = state0;   // start in idle state on RESET
    lpm_mode = mode0; // start in idle state on RESET 
    sysConfig();

    while(1){
	    switch(state){
	        case state0:
			stop_timers();
	            	TIMER_SERVO();
	            	pwm_servo(90);
                	DelayMs_loop(4);
	            	stop_timers();
                	cursor_off;
                	enable_interrupts();
                	enterLPM(lpm_mode);
                	break;       
				 
	        case state1:
	                angle=0;
	                TIMER_SERVO();        // config TIMER0 for servo
	                pwm_servo(angle);
	                DelayMs_loop(3);
	                while (angle < 180){
	                    pwm_servo(angle);
	                    DelayMs(4500);
	                    TIMER_DISTANCE();
	                    capture();
	                    enterLPM(lpm_mode);
	                    angle += AngleSpace;
	                }
	                stop_timers();
	                transmit_dist_arr();
	                state = state0;
	                break;        

            	case state2:
	                flag_user_angle = 0;
	                while (flag_user_angle == 0){  // get angle from PC side 
	                    enterLPM(lpm_mode);
	                    flag_user_angle = update_flag_user_angle();
	                }
	                angle = get_user_angle();
	                TIMER_SERVO();
	                pwm_servo(angle);         // 0.6ms + user_angle * 11;  11 = 1 degree
	                DelayMs_loop(3);
	                stop_timers();
	                TIMER_DISTANCE();         // initial timer register for telemeter
	                capture();                // set capture mode and enable interrupt
	                while (state == state2){  // calculate distance loop and send when changed
	                    capture();
	                    enterLPM(lpm_mode);
	                    send_distance();
	                }
	                break;

            	case state3:
	                angle=0;
	                TIMER_SERVO();  // config TIMER0 for servo
	                pwm_servo(angle);
	                DelayMs_loop(3);
	                while (angle < 180){
	                    TIMER_SERVO();
	                    pwm_servo(angle);
	                    DelayMs(45000);
	                    stop_timers();
	                    ADCconfig_LDR1();
	                    enterLPM(lpm_mode);
	                    LDR_value();
	                    ADCconfig_LDR2();
	                    enterLPM(lpm_mode);
	                    LDR_value();
	                    angle += AngleSpace;
	                }
	                stop_timers();
	                transmit_ldr_arr();
	                transmit_num(999);
	                state = state0;
	                break;

	        case state5:
	            	set_flash_ptr(); // set address in flash for txt file
	            	write_Seg();
	            	state = state0;
                	break;

	        case state6:
	            	enterLPM(lpm_mode);
	            	scirpt_number = UCA0RXBUF;

	            	if (scirpt_number == 0x31){
	                	script_number_int = 1;
	            	}
	            	else if (scirpt_number == 0x32){
                    		script_number_int = 2;
                	}
	            	else if (scirpt_number == 0x33){
                    		script_number_int = 3;
                	}

	            	script(script_number_int);
                	state = state0;
	            	break;

	        case state7:
                	stop_timers();
                	state = state0;
                	break;
            }
    }
}  
