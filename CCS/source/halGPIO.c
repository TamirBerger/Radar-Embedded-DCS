#include  "../header/halGPIO.h"     // private library - HAL layer
#include  "../header/api.h"     // private library - HAL layer

#define DistSize 60

// state1
int dist_size_var = DistSize;
unsigned int distance=0;
unsigned int diff_time=0;
unsigned int i=0;
int temp[2];
long LDR_val[2];
unsigned int j=0;
int dist_arr[DistSize];
unsigned int idx = 0;
char str_num[4];

// state2
unsigned int arr_angle_ascii[4];
unsigned int q = 0;
unsigned int angle_digits = 0;
unsigned int user_angle;
int flag_user_angle = 0;
int prev_avg = 0;  // used in 'send_distance()'

//state 3
int treshold_ldr = 961;

// state5
int write_flash_flag;
char *Flash_ptr = (char*)0x1000;
char *Flash_script_start[4];
char *Flash_script_end;
int script_num = 0;

//script
char *task_ptr;
char OPC;
int counter = 0;
int number_from_user = 0;
int operand1;
int operand2;
int flag_opc6 = 0;

char* str_delay = "enter X_ms";
unsigned int arr_Xms_ascii[5];
unsigned int X_ms_digits = 0;
unsigned int X_ms = 500;
char* str_TX;
int flag;

long prev_LDR_val = 0;
long counter_adc = 100;
long adc_sum = 0;


void transmit_ldr_arr(){
    int k;

    for(k=0; k < DistSize; k++){
        if (dist_arr[k] < treshold_ldr){
            transmit_num(k);    // index
            enterLPM(lpm_mode);
            transmit_num(dist_arr[k]);
            enterLPM(lpm_mode);
        }
    }
}

//******************************************************************
//                      SCRIPT
//******************************************************************
void script(int scirpt_number)
{
    //int script_sel = scirpt_number - '0';
    task_ptr = Flash_script_start[scirpt_number-1]; //address of the beginning of the script
    task_ptr++; // equal to s
    task_ptr++; // equal to 0
    OPC = *task_ptr++; // get the first opc
    int angle_script = 0;
    int k =0;
    int j =0;
    X_ms = 500;

    while ( (*task_ptr != 's') || (*task_ptr != 'f' ) || (*task_ptr != 'l') ) {  // s - begining of script , f - end of all scripts

        if (OPC == 0x31){ // opc = 1 inc_lcd
            counter = 0;
            lcd_home();
            operand1 = AsciToint1(*task_ptr++, *task_ptr++); // convert the operand from ASCI to int
            while (counter <= operand1){
                delay_func(X_ms); // include TimerA config
                counterup(counter);
                counter++;
                if (counter == operand1)
                    break;
            }
        }

        if (OPC == 0x32){ // opc = 2 dec_lcd
            operand1 = AsciToint1(*task_ptr++, *task_ptr++); // convert the operand from ASCI to int
            counter = operand1;
            lcd_home();
            while (counter >= 0){
                delay_func(X_ms); // include TimerA config
                counterdown(counter);
                counter--;
                if (counter == 0)
                    break;
            }
        }

        if (OPC == 0x33) { // opc = 3 rra_lcd
               operand1 = AsciToint1(*task_ptr++, *task_ptr++);
               k=0;
               lcd_home();
               while (k < 31){
                   lcd_clear();
                   j=0;
                   for (j = 0; j < k; j++){
                       if (j == 15){
                           lcd_new_line;
                       }
                       lcd_cursor_right();
                   }

                   lcd_data(operand1);
                   delay_func(X_ms); // include TimerA config
                   k++;
               }
        }

        if (OPC == 0x34) { // opc = 4 set_dealy
                operand1 = AsciToint1(*task_ptr++, *task_ptr++); // convert the operand from ASCI to int
                X_ms = 10 * operand1;
        }

        if (OPC == 0x35){ // opc = 5 clear lcd
                lcd_clear();
        }

        if (OPC == 0x36){ // opc = 6 servo_deg
           
            transmit_num(6);
            enterLPM(lpm_mode);     // wait for ack 'a'
            operand1 = AsciToint1(*task_ptr++, *task_ptr++); // convert the operand from ASCI to int
            TIMER_SERVO();
            pwm_servo(operand1);   // 0.6ms + user_angle * 11;  11 = 1 degree
            DelayMs_loop(3);
            stop_timers();
            TIMER_DISTANCE();         // initial timer register for telemeter
            idx = 0;
            prev_avg = 0;
            while (flag_opc6 == 0){
                capture();                // set capture mode and enable interrupt
                enterLPM(lpm_mode);
                send_distance();
            }
            stop_timers();
            flag_opc6 = 0;
            enterLPM(lpm_mode);
        }

        if (OPC == 0x37){ // opc = 7 servo_scan
            transmit_num(7);
            enterLPM(lpm_mode);     // wait for ack 'a'
            operand1 = AsciToint1(*task_ptr++, *task_ptr++); // convert the operand from ASCI to int
            operand2 = AsciToint1(*task_ptr++, *task_ptr++); // convert the operand from ASCI to int
            dist_size_var = (operand2 - operand1) / (180 / DistSize);
            transmit_num(operand1);
            enterLPM(lpm_mode);     // wait for ack 'a'
            transmit_num(operand2);
            enterLPM(lpm_mode);     // wait for ack 'a'
            angle_script = operand1;
            TIMER_SERVO();        // config TIMER0 for servo
            pwm_servo(angle_script);
            DelayMs_loop(2);
            while (angle_script < operand2){
                pwm_servo(angle_script);
                DelayMs(65000);
                TIMER_DISTANCE();
                capture();
                enterLPM(lpm_mode);
                angle_script += 3;
            }
            stop_timers();
            transmit_dist_arr();
        }

        if (OPC == 0x38){ // opc = 8 sleep
            transmit_num(8);
            enterLPM(lpm_mode);
            state = state0;
            break;
        }

    task_ptr++; // equal to 0
    OPC = *task_ptr++; // get the first opc
    }
}

int AsciToint1(char str1, char str2) {
    int op1 = 0;
    int op2 = 0;

    if(str1 < '9')
        op1 = (str1 - 48)*16;
    else
        op1 = (str1 - 55)*16;

    if(str2 < '9')
        op2 = (str2 - 48);
    else
        op2 = (str2 - 55);

    return op1 + op2;
}

//******************************************************************
//                      SERVO
//******************************************************************
void pwm_servo(int angle){
    /*
     * 0 deg = 0.6ms;   180 deg = 2.5ms;
     * 2.5 - 0.6 = 1.9; 1.9 deg / 60 = 0.0316 deg
     *
     */
    TA0CCR1 = 629 + angle * 11;  // 0.6msec = 0 deg --> 60 steps each step Increases in 0.0316ms
    //DelayMs(65000);
    return;
}


void capture(){
    //capture echo signal from HR-sc
    TA1CCTL2 = CAP; //CAPTURE MODE
    TA1CCTL2 |= CCIE; // intarupt enable
    TA1CCTL2 |= SCS; // Capture sychronize
    TA1CCTL2 |= CCIS_0; // Capture input select: 1 - CCIxB - P2.2 output
    TA1CCTL2 |= CM_3; // capture on both rising edge and falling edge
}


//******************************************************************
//                      TIMER
//******************************************************************
  // ==============        interrupts      ======================
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
   switch(lpm_mode){
       case mode0:
        LPM0_EXIT; // must be called from ISR only
        break;

       case mode1:
        LPM1_EXIT; // must be called from ISR only
        break;

       case mode2:
        LPM2_EXIT; // must be called from ISR only
        break;

       case mode3:
        LPM3_EXIT; // must be called from ISR only
        break;

       case mode4:
        LPM4_EXIT; // must be called from ISR only
        break;
   }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A1_VECTOR  //timer 1 low interrupt vector
__interrupt void Timer1(void)
#elif defined(__GNUC__)
  void __attribute__((interrupt(TIMER1_A1_VECTOR))) Timer1(void)
#else
#error Compiler not supported!
#endif
  {
    switch (TA1IV){
        case  2: break;              // CCIFG1

        case  4:
            TA1CCTL2 &= ~CCIE;   // intarupt unenable
            temp[i] = TA1CCR2;   //
            i++;
            TA1CCTL1 &= ~CCIFG ; // Reset Capture/compare interrupt flag
            TA1CCTL2 |= CCIE;    // intarupt enable
            if (i==2) {
                diff_time=temp[i-1]-temp[i-2];
                dist_arr[idx] = diff_time/58; //clc distance from echo
                i=0;
                idx++;
                if (idx == dist_size_var){ // check
                    idx = 0;
                }
                TA1CCTL2 &= ~CCIE;  // intarupt unenable

                switch(lpm_mode){
                    case mode0:
                        LPM0_EXIT; // must be called from ISR only
                        break;

                    case mode1:
                        LPM1_EXIT; // must be called from ISR only
                        break;

                    case mode2:
                        LPM2_EXIT; // must be called from ISR only
                        break;

                    case mode3:
                        LPM3_EXIT; // must be called from ISR only
                        break;

                    case mode4:
                        LPM4_EXIT; // must be called from ISR only
                        break;
                }
            } 
            break;

	    case 10:
	        TA1CTL  &= ~ TAIFG;
	        TA1CTL |= MC_0;    // Stop Mode Timer_A
	        TA1CCR1=0;
	        TA1R = 0;
	        i=0;
	        TIMER_DISTANCE();
	        break;
    }
  }


void transmit_dist_arr(){
    int k;
    for(k=0; k < dist_size_var; k++){
        transmit_num(dist_arr[k]);
        enterLPM(lpm_mode);
    }
    dist_size_var = DistSize;
}

//-------------------------------------------------------------------------------------
//           erase segment 0x1000 in flash
//-------------------------------------------------------------------------------------
void erase_seg(){
      int i;

      FCTL3 = FWKEY;                            // Clear Lock bit
      FCTL1 = FWKEY + WRT;                      // Set Erase bit
      for (i=0; i<64; i++){
        *Flash_ptr++ = 0xFF;                    // Write value to flash
      }
      FCTL1 = FWKEY;                            // Clear WRT bit
      FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

//-------------------------------------------------------------------------------------
//           write to flash
//-------------------------------------------------------------------------------------
void write_Seg(){
    if (CALBC1_1MHZ==0xFF){                   // If calibration constant erased
      while(1);                               // do not load, trap CPU!!
    }
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1MHz
    DCOCTL = CALDCO_1MHZ;

    // erase segment
    FCTL2 = FWKEY + FSSEL0 + FN1;  // MCLK/3 for Flash Timing Generator (should i reset that after?)
    FCTL1 = FWKEY + ERASE;         // Set Erase bit
    FCTL3 = FWKEY;                 // Clear Lock bit
    *Flash_ptr = 0;                // Dummy write to erase Flash segment

    // write to segment
    FCTL1 = FWKEY + WRT;           // Set WRT bit for write operation
   while (write_flash_flag == 0){
       enterLPM(lpm_mode);

       if (UCA0RXBUF =='s'){
           Flash_script_start[script_num] = Flash_ptr;
           if (script_num == 3){
               script_num = 0;
           }
           else{
               script_num++;
           }
       }
       if (UCA0RXBUF == 'f'){
           write_flash_flag = 1;
           return;
       }
       if (UCA0RXBUF == 'l'){
           write_flash_flag = 1;
       }

      *Flash_ptr++ = UCA0RXBUF;   // Write value to flash
   }

   FCTL1 = FWKEY;                 // Clear WRT bit
   FCTL3 = FWKEY + LOCK;          // Set LOCK bit
}

//-------------------------------------------------------------------------------------
//           set address to store script txt file in flash
//-------------------------------------------------------------------------------------
void set_flash_ptr(){
    Flash_ptr = (char*)(0x1000 + (script_num*0x40));
    write_flash_flag = 0;
}

//-------------------------------------------------------------------------------------
//           get write_flash_flag
//-------------------------------------------------------------------------------------
int get_write_flash_flag(){
    return write_flash_flag;
}

//-------------------------------------------------------------------------------------
//           transmit 3 digits num (distance) to PC
//-------------------------------------------------------------------------------------
void transmit_num(int num){
    intToDecStr(num, str_num);
    transmit_str(str_num);
}

//-------------------------------------------------------------------------------------
//           transmit str to PC
//-------------------------------------------------------------------------------------
void transmit_str(char* str){
    str_TX = str;
    i = 0;
    IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
    UCA0TXBUF = str_TX[i];
    IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
}

//-------------------------------------------------------------------------------------
//           transmit str to PC
//           calculate avg of last 'distSize' - 1 samples, and send if there is a change from the previous avg.
//-------------------------------------------------------------------------------------
void send_distance(){   // state 2
    // sensitivity of the sensor: end new distance at change of 2 cm
    int new_avg = 0;
    int r;

    if (idx == (DistSize - 1)){
        for (r = 0; r < idx; r++){
            new_avg += dist_arr[r];
        }
        new_avg /= (DistSize -1);

        if (new_avg - prev_avg > 1 || new_avg - prev_avg < -1){
            transmit_num(new_avg);
            enterLPM(lpm_mode);     // wait for ack 'a'
            prev_avg = new_avg;

            if (state == state6){
                flag_opc6 = 1;
                //transmit_num(0);
            }
        }
        idx = 0;
    }
}

//-------------------------------------------------------------------------------------
//           get user_angle
//-------------------------------------------------------------------------------------
unsigned int get_user_angle(){
    return user_angle;
}

//-------------------------------------------------------------------------------------
//           recieve_angle function
//-------------------------------------------------------------------------------------
void recieve_angle(){
    int j;
    user_angle = 0;

    for(j = 0; j < angle_digits; j++){
        user_angle = user_angle * 10 + (arr_angle_ascii[j] - 48);
    }
}

//-------------------------------------------------------------------------------------
//           update if PC finished to send user angle in state 2.
//           if finished -> flag = 1
//-------------------------------------------------------------------------------------
int update_flag_user_angle(){
    return flag_user_angle;
}

//*********************************************************************
//            TX ISR
//*********************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCI0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    UCA0TXBUF = str_TX[i++];              // TX next character

      if (str_TX[i] == '\0'){             // TX over?
        IE2 &= ~UCA0TXIE;
      }

}

//*********************************************************************
//            RX ISR
//*********************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    if (UCA0RXBUF == 'a'){  // ack -> wake up and continue
                            // state1: for each distance measurement that sent to PC
                            // state2: after sent update distance to PC
        LPM0_EXIT;  // must be called from ISR only
        return;
    }

    else if (state == state2){
        if(UCA0RXBUF == 'e'){   // end of state2
            state = state0;
            return;
        }
        // get angle from user
        flag_user_angle = 0;
        arr_angle_ascii[q] = UCA0RXBUF;
        q++;

        if (arr_angle_ascii[q-1] == '\n'){  // full angle data received
            angle_digits = q - 1;           // angle_digits = len of arr_angle_ascii
            q = 0;
            recieve_angle();
            flag_user_angle = 1;
        }
    }

    else if (state == state5){   // script mode
        if (OPC == 6){
            flag_opc6 = 1;
        }

        LPM0_EXIT;  // must be called from ISR only
        return;
    }

    else if (state == state4){
        arr_Xms_ascii[q] = UCA0RXBUF;
        q++;

        if (arr_Xms_ascii[q-1] == '\n'){
            X_ms_digits = q - 1;  // X_ms_digits = len of arr_Xms_ascii
            q = 0;
            recieve_X_ms();
            state = state0;
        }
    }
    else if (state == state6){
        LPM0_EXIT;  // must be called from ISR only
        return;

    }
    else{

        if(UCA0RXBUF == '1'){
            state = state1;
        }
        else if(UCA0RXBUF == '2'){
            state = state2;
        }
        else if(UCA0RXBUF == '3'){
            state = state3;
        }
        else if(UCA0RXBUF == '4'){
            state = state4;
        }
        else if(UCA0RXBUF == '5'){
            state = state5;
        }
        else if(UCA0RXBUF == '6'){
            state = state6;
        }
    } // end else

    //---------------------------------------------------------------------
    //            Exit from a given LPM
    //---------------------------------------------------------------------
        switch(lpm_mode){

            case mode0:
             LPM0_EXIT; // must be called from ISR only
             break;

            case mode1:
             LPM1_EXIT; // must be called from ISR only
             break;

            case mode2:
             LPM2_EXIT; // must be called from ISR only
             break;

            case mode3:
             LPM3_EXIT; // must be called from ISR only
             break;

            case mode4:
             LPM4_EXIT; // must be called from ISR only
             break;

        }
 } // end RX interrupt


//******************************************************************
  //                      ADC
//******************************************************************

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  switch(lpm_mode){
        case mode0:
         LPM0_EXIT; // must be called from ISR only
         break;

        case mode1:
         LPM1_EXIT; // must be called from ISR only
         break;

        case mode2:
         LPM2_EXIT; // must be called from ISR only
         break;

        case mode3:
         LPM3_EXIT; // must be called from ISR only
         break;

        case mode4:
         LPM4_EXIT; // must be called from ISR only
         break;
    }
}

//-------------------------------------------------------------------------------------
//           get X_ms
//-------------------------------------------------------------------------------------
int get_X_ms(){
    return X_ms;
}

//-------------------------------------------------------------------------------------
//           recieve_X_ms function
//-------------------------------------------------------------------------------------
void recieve_X_ms(){
    int j;
    X_ms = 0;

    for(j = 0; j < X_ms_digits; j++){
        X_ms = X_ms * 10 + (arr_Xms_ascii[j] - 48);
    }
}

//--------------------------------------------------------------------
//             System Configuration
//--------------------------------------------------------------------
void sysConfig(void){
    GPIOconfig();
    lcd_init();
    initFlash();
}

//---------------------------------------------------------------------
//            DelayMs_loop
//---------------------------------------------------------------------
void DelayMs_loop(unsigned int cnt)
{
    unsigned char i;
    for(i=cnt ; i>0 ; i--) DelayMs(65000); // tha command asm("nop") takes raphly 1usec]
}

//---------------------------------------------------------------------
//            Timer based Delay function. range: 1ms - 9999ms
//---------------------------------------------------------------------
void delay_func(int delay){

    while (delay){

        if (delay >= 500){
            TIMER_A_config(65535); // 500 ms
            delay -= 500;
        }

        else{
        TIMER_A_config(131 * delay); // 1ms * delay ms
        delay = 0;
        }
        enterLPM(lpm_mode);
    }
    stop_timers();
}

//---------------------------------------------------------------------
//            Polling based Delay function
//---------------------------------------------------------------------
void delay(unsigned int t){  // t[msec]
    volatile unsigned int j;
    for(j=t; j>0; j--);
}

//******************************************************************
// Delay Usec functions
//******************************************************************
void DelayUs(unsigned int cnt)
{
    unsigned char i;
    for(i=cnt ; i>0 ; i--) asm("noP"); // tha command asm("nop") takes raphly 1usec]
}

//******************************************************************
// Delay msec functions
//******************************************************************
void DelayMs(unsigned int cnt){

    unsigned char i;
        for(i=cnt ; i>0 ; i--) DelayUs(1000); // tha command asm("nop") takes raphly 1usec
}

//---------------------------------------------------------------------
//            Enter from LPM0 mode
//---------------------------------------------------------------------
void enterLPM(unsigned char LPM_level){
    if (LPM_level == 0x00)
      _BIS_SR(LPM0_bits);     /* Enter Low Power Mode 0 */
        else if(LPM_level == 0x01)
      _BIS_SR(LPM1_bits);     /* Enter Low Power Mode 1 */
        else if(LPM_level == 0x02)
      _BIS_SR(LPM2_bits);     /* Enter Low Power Mode 2 */
    else if(LPM_level == 0x03)
      _BIS_SR(LPM3_bits);     /* Enter Low Power Mode 3 */
        else if(LPM_level == 0x04)
      _BIS_SR(LPM4_bits);     /* Enter Low Power Mode 4 */
}

//---------------------------------------------------------------------
//            Enable interrupts
//---------------------------------------------------------------------
void enable_interrupts(){
  _BIS_SR(GIE);
}

//---------------------------------------------------------------------
//            Disable interrupts
//---------------------------------------------------------------------
void disable_interrupts(){
  _BIC_SR(GIE);
}

//******************************************************************
  //                      LCD FUNCTIONS
  //****************************************************************

  //****************************************************************
// send a command to the LCD
//******************************************************************
void lcd_cmd(unsigned char c){  
	LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h

	if (LCD_MODE == FOURBIT_MODE){
		LCD_DATA_WRITE &= ~OUTPUT_DATA;// clear bits before new write
                LCD_DATA_WRITE |= ((c >> 4) & 0x0F) << LCD_DATA_OFFSET;
		lcd_strobe();
                LCD_DATA_WRITE &= ~OUTPUT_DATA;
    		LCD_DATA_WRITE |= (c & (0x0F)) << LCD_DATA_OFFSET;
		lcd_strobe();
	}
	else{
		LCD_DATA_WRITE = c;
		lcd_strobe();
	}
}

//******************************************************************
// send data to the LCD
//******************************************************************
void lcd_data(unsigned char c){        
	LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h

	LCD_DATA_WRITE &= ~OUTPUT_DATA;       
	LCD_RS(1);
	if (LCD_MODE == FOURBIT_MODE){
    		LCD_DATA_WRITE &= ~OUTPUT_DATA;
                LCD_DATA_WRITE |= ((c >> 4) & 0x0F) << LCD_DATA_OFFSET;  
		lcd_strobe();		
                LCD_DATA_WRITE &= (0xF0 << LCD_DATA_OFFSET) | (0xF0 >> 8 - LCD_DATA_OFFSET);
                LCD_DATA_WRITE &= ~OUTPUT_DATA;
		LCD_DATA_WRITE |= (c & 0x0F) << LCD_DATA_OFFSET; 
		lcd_strobe();
	}
	else{
		LCD_DATA_WRITE = c;
		lcd_strobe();
	}
	LCD_RS(0);   
}

//******************************************************************
// initialize the LCD
//******************************************************************
void lcd_init(){
	char init_value;

	if (LCD_MODE == FOURBIT_MODE) init_value = 0x3 << LCD_DATA_OFFSET;
    else init_value = 0x3F;
	
	LCD_RS_DIR(OUTPUT_PIN);
	LCD_EN_DIR(OUTPUT_PIN);
	LCD_RW_DIR(OUTPUT_PIN);
    LCD_DATA_DIR |= OUTPUT_DATA;
    LCD_RS(0);
	LCD_EN(0);
	LCD_RW(0);
        
	DelayMs(15);
    LCD_DATA_WRITE &= ~OUTPUT_DATA;
	LCD_DATA_WRITE |= init_value;
	lcd_strobe();
	DelayMs(5);
    LCD_DATA_WRITE &= ~OUTPUT_DATA;
	LCD_DATA_WRITE |= init_value;
	lcd_strobe();
	DelayUs(200);
    LCD_DATA_WRITE &= ~OUTPUT_DATA;
	LCD_DATA_WRITE |= init_value;
	lcd_strobe();
	
	if (LCD_MODE == FOURBIT_MODE){
		LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h
        LCD_DATA_WRITE &= ~OUTPUT_DATA;
		LCD_DATA_WRITE |= 0x2 << LCD_DATA_OFFSET; // Set 4-bit mode
		lcd_strobe();
		lcd_cmd(0x28); // Function Set
	}
    else lcd_cmd(0x3C); // 8bit,two lines,5x10 dots 
	
	lcd_cmd(0xF); //Display On, Cursor On, Cursor Blink
	lcd_cmd(0x1); //Display Clear
	lcd_cmd(0x6); //Entry Mode
	lcd_cmd(0x80); //Initialize DDRAM address to zero
}
 
//******************************************************************
// lcd strobe functions
//******************************************************************
void lcd_strobe(){
  LCD_EN(1);
  asm("nOp");
  asm("Nop");
  LCD_EN(0);
}

void LDR_value(){
    long LDR_val[2];
    long LDR_val_avg = 0;
    long adc_sum = 0;
    long counter_adc = 100;

    while (counter_adc != 0){
     ADC10CTL0 &= ~ENC;

     adc_sum += ADC10MEM;
     counter_adc--;
     ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
     if (counter_adc > 0)
         enterLPM(lpm_mode);
    }

    ADC10CTL0 &= ~ENC;
    counter_adc = 100;
    LDR_val[j] = adc_sum / counter_adc;
    adc_sum = 0;
    j++;

    if (j == 2){
        LDR_val_avg = (LDR_val[0] + LDR_val[1]) / 2;
        dist_arr[idx] = LDR_val_avg;
        idx++;
        if (idx == DistSize){
            idx = 0;
        }
        j = 0;
    }
}
