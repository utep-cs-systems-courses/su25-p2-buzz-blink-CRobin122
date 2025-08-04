//Alternate LEDs from Off, Green, and Red
#include <msp430.h>
#include "libTimer.h"
#include "led.h"

int main(void) {
  P1DIR |= LEDS;
  P1OUT &= ~LED_GREEN;
  P1OUT |= LED_RED;

  configureClocks();		/* setup master oscillator, CPU & peripheral clocks */
  enableWDTInterrupts();	/* enable periodic interrupt */
  
  or_sr(0x18);		/* CPU off, GIE on */
}

char secCount = 0;
char greenLimit = 0;
char greenCount = 0;

void
oncePerSec()
{
  static char redState = 0;
  switch (redState) {
  case 0:
    redState = 1; P1OUT |= LED_RED; break;
  case 1:
    redState = 0; P1OUT &= ~LED_RED; break;
  }
  greenLimit ++;
  if (greenLimit == 10)
    greenLimit = 0;
}

char ledState = 0;

void
__interrupt_vec(WDT_VECTOR) WDT()	/* 250 interrupts/sec */
{
  secCount ++;
  if(secCount == 250){
    secCount = 0;
    oncePerSec();
  }
  
  if (greenCount == 0)
    P1OUT |= LED_GREEN;
  greenCount ++;
  if (greenCount >= greenLimit){
    P1OUT &= ~LED_GREEN;
    greenCount =0;
  }
  
  switch (ledState){
  case 0: P1OUT |= LED_GREEN; ledState = 1; break;
  case 1: P1OUT &= ~LED_GREEN; ledState = 2; break;
  case 2: ledState = 0;
  }
} 

