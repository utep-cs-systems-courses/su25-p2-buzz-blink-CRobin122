
//Enhanced LED Control - Implements all suggested activities
#include <msp430.h>
#include "libTimer.h"
#include "led.h"

int main(void) {
  P1DIR |= LEDS;
  P1OUT &= ~LEDS;  // Start with both LEDs off

  configureClocks();		/* setup master oscillator, CPU & peripheral clocks */
  enableWDTInterrupts();	/* enable periodic interrupt */
  
  or_sr(0x18);			/* CPU off, GIE on */
}

// Green LED state variables
int greenBlinkLimit = 1;     // duty cycle = 1/greenBlinkLimit (start dim)
int greenBlinkCount = 0;     // cycles 0...greenBlinkLimit-1
int greenSecondCount = 0;    // state var for green LED timing
int greenDirection = 1;      // 1 = getting brighter, -1 = getting dimmer

// Red LED state variables  
int redBlinkLimit = 7;       // duty cycle = 1/redBlinkLimit (start bright)
int redBlinkCount = 0;       // cycles 0...redBlinkLimit-1
int redSecondCount = 0;      // state var for red LED timing
int redDirection = -1;       // 1 = getting brighter, -1 = getting dimmer

// Speed control
int speedCount = 0;
int speedLimit = 125;        // Change brightness every 0.5 seconds (faster than original)

void
__interrupt_vec(WDT_VECTOR) WDT()	/* 250 interrupts/sec */
{
  // Handle GREEN LED blinking with PWM-like behavior
  greenBlinkCount++;
  if (greenBlinkCount >= greenBlinkLimit) { 
    greenBlinkCount = 0;
    P1OUT |= LED_GREEN;      // on for 1 interrupt period
  } else {
    P1OUT &= ~LED_GREEN;     // off for blinkLimit - 1 interrupt periods
  }

  // Handle RED LED blinking with PWM-like behavior (independent pattern)
  redBlinkCount++;
  if (redBlinkCount >= redBlinkLimit) { 
    redBlinkCount = 0;
    P1OUT |= LED_RED;        // on for 1 interrupt period
  } else {
    P1OUT &= ~LED_RED;       // off for redBlinkLimit - 1 interrupt periods
  }

  // Control brightness change speed
  speedCount++;
  if (speedCount >= speedLimit) {
    speedCount = 0;
    
    // Update GREEN LED brightness (dim-to-bright-to-dim cycle)
    greenSecondCount++;
    if (greenSecondCount >= 1) {  // Change green brightness
      greenSecondCount = 0;
      
      greenBlinkLimit += greenDirection;
      
      // Reverse direction at limits (1 = brightest, 7 = dimmest)
      if (greenBlinkLimit >= 7) {
        greenBlinkLimit = 7;
        greenDirection = -1;     // start getting brighter
      } else if (greenBlinkLimit <= 1) {
        greenBlinkLimit = 1;
        greenDirection = 1;      // start getting dimmer
      }
    }
    
    // Update RED LED brightness (opposite pattern to green)
    redSecondCount++;  
    if (redSecondCount >= 1) {   // Change red brightness
      redSecondCount = 0;
      
      redBlinkLimit += redDirection;
      
      // Reverse direction at limits (opposite of green)
      if (redBlinkLimit >= 7) {
        redBlinkLimit = 7;
        redDirection = -1;       // start getting brighter  
      } else if (redBlinkLimit <= 1) {
        redBlinkLimit = 1;
        redDirection = 1;        // start getting dimmer
      }
    }
  }
}

