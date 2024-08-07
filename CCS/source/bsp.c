#include "../header/bsp_msp430x4xx.h"

//-----------------------------------------------------------------------------  
//           GPIO congiguration
//-----------------------------------------------------------------------------
void GPIOconfig(void) {
    WDTCTL = WDTHOLD | WDTPW;		// Stop WDT
	_BIS_SR(GIE);                   // enable interrupts globally

// clock configuration
    P2SEL |= (BIT6 + BIT4 + BIT2);   // GPIO P2.6, 2.4, 2.2 (pwm, echo, trigger)
    P2SEL &= ~BIT7;                  // GPIO P2.7
    P2SEL2 &= ~(BIT7+BIT6);          // GPIO P2.7,2.6 ??
    P2DIR |= (BIT2 + BIT6);          //  P2.2,2.6 OUTPUT (TA1.1,TA0.1 Output) (trigger, pwm)
    P2DIR &= ~BIT4;                  // P2.4 INPUT (echo)

// LCD CNTL
    LCD_CNTL_SEL &= ~LCD_CNTL_BITS;
// LCD DATA
    LCD_DATA_SEL &= ~OUTPUT_DATA;

// UART
    if (CALBC1_1MHZ==0xFF){                  // If calibration constant erased
        while(1);                               // do not load, trap CPU!!
    }
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1OUT &= ~0x03;
    UCA0CTL1 |= UCSSEL_2;                     // CLK = SMCLK
    UCA0BR0 = 104;                           //
    UCA0BR1 = 0x00;                           //
    UCA0MCTL = UCBRS0;               //
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

//-------------------------------------------------------------------------------------
//            initial flash memory
//-------------------------------------------------------------------------------------
void initFlash(void){
    if (CALBC1_1MHZ==0xFF){                  // If calibration constant erased
        while(1);                            // do not load, trap CPU!!
    }
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1MHz
    DCOCTL = CALDCO_1MHZ;
}

void stop_timers(void){
  ADC10CTL0 &= ~ADC10ON;
  TA1CTL = MC_0;
  TA0CTL = MC_0;
}

//-------------------------------------------------------------------------------------
//            ADC congiguration
//-------------------------------------------------------------------------------------
void ADCconfig_LDR1(void) {
    ADC10CTL1 = INCH_3 + ADC10DIV_3;                      // Channel 3 (see page 6 on data sheet), ADC10CLK/3
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;  // Vcc & Vss as reference, Sample and hold for 64 Clock cycles, ADC on, ADC interrupt enable
    ADC10AE0 |= BIT3;
    ADC10CTL0 |= ENC + ADC10SC;               // Sampling and conversion start
    P1SEL |= BIT3;
}

void ADCconfig_LDR2(void) {
    ADC10CTL1 = INCH_0 + ADC10DIV_3;                      // Channel 3 (see page 6 on data sheet), ADC10CLK/3
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;  // Vcc & Vss as reference, Sample and hold for 64 Clock cycles, ADC on, ADC interrupt enable
    ADC10AE0 |= BIT0;
    ADC10CTL0 |= ENC + ADC10SC;               // Sampling and conversion start
    P1SEL |= BIT0;
}

void TIMER_A_config(unsigned int X_ms){
  TACTL = TASSEL_2 + MC_3 + ID_3 + TACLR;      // SMCLK, UP MODE, divide 8, counter clear
  TA0CCTL1 = OUTMOD_0 ;
  TACCTL0 = CCIE;  // Capture/compare interrupt enable
  TACCR0 = X_ms;
}

void TIMER_config_state3(void) {
  TA1CTL = TACLR;
  TA1CTL = TASSEL_2 + MC_1 + ID_3; // SMCLK, UP MODE
  TA1CCTL2 = OUTMOD_4; // p2.4 output, toggle
}

void TIMER_SERVO(void) {
    TACCTL0 &= ~CCIE;  // Capture/compare interrupt enable
    TA0CTL = TACLR + ID_0 + TASSEL_2 + MC_1;
    TA0CCR0 = 26214;        // T = 25 msec
    TA0CCTL1 = OUTMOD_7 ;   // on until TA0CCR1. of from TA0CCR1 to TA0CCR0
}

void TIMER_DISTANCE(void) {
  TA1CTL  = TASSEL_2;  // SMCLK
  TA1CCTL1 = OUTMOD_7; // on until TA0CCR1. of from TA0CCR1 to TA0CCR0
  TA1CCR0 = 62926;     // 60 ms(space) + 10 usec(trigger); old val was 65535
  TA1CCR1 = 0x000A;    // 10 usec
  TA1CTL  |= MC_1;     // UP MODE
  TA1CTL  |= TAIE;     // enable intarupt
}
