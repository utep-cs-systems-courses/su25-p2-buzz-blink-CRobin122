#include <msp430.h>
#include "libTimer.h"
#include "led.h"

/*
 * Main function - Initialize hardware and enter low power mode
 */
int main(void) 
{
    // Configure LED pins as outputs
    P1DIR |= LEDS;
    
    // Initial LED state: Green off, Red on
    P1OUT &= ~LED_GREEN;    // Turn off green LED
    P1OUT |= LED_RED;       // Turn on red LED
    
    // Initialize system clocks and watchdog timer interrupt
    configureClocks();      /* Setup master oscillator, CPU & peripheral clocks */
    enableWDTInterrupts();  /* Enable periodic interrupt (250 Hz) */
    
    // Enter low power mode with interrupts enabled
    or_sr(0x18);            /* CPU off, GIE (Global Interrupt Enable) on */
}

/*
 * Hardware abstraction function for green LED control
 * 
 * Parameters:
 *   on - 1 to turn LED on, 0 to turn LED off
 */
void greenControl(int on)
{
    if (on) {
        P1OUT |= LED_GREEN;     // Set bit to turn on LED
    } else {
        P1OUT &= ~LED_GREEN;    // Clear bit to turn off LED
    }
}

/*
 * Blink State Machine
 * Controls LED brightness using PWM-style duty cycle modulation
 */
static int blinkLimit = 5;  // Duty cycle = 1/blinkLimit (smaller = brighter)

void blinkUpdate() 
{
    /*
     * Called every 1/250 seconds (4ms) to create PWM-like blinking
     * Creates duty cycle of 1/blinkLimit by turning LED on for 1 cycle
     * and off for (blinkLimit-1) cycles
     */
    static int blinkCount = 0;  // Tracks current position in blink cycle
    
    blinkCount++;
    
    if (blinkCount >= blinkLimit) {
        blinkCount = 0;         // Reset cycle counter
        greenControl(1);        // Turn LED on for 1 interrupt period
    } else {
        greenControl(0);        // Turn LED off for remaining periods
    }
}

/*
 * Brightness Control Logic
 * Changes the duty cycle once per second to create brightness variation
 */
void oncePerSecond() 
{
    /*
     * Gradually reduces LED brightness by increasing blinkLimit
     * Cycles from bright (blinkLimit=1) to dim (blinkLimit=7)
     * Then resets to start the cycle again
     */
    blinkLimit++;               // Reduce duty cycle (make dimmer)
    
    if (blinkLimit >= 8) {      // Prevent going beyond dimmest setting
        blinkLimit = 0;         // Reset to brightest (will become 1 next cycle)
    }
}

/*
 * Second Timer State Machine
 * Generates a once-per-second event from 250Hz interrupt
 */
void secondUpdate()  
{
    /*
     * Called every 1/250 seconds to count up to one full second
     * Triggers brightness change when a second has elapsed
     */
    static int secondCount = 0; // Counts interrupt periods (0 to 249)
    
    secondCount++;
    
    if (secondCount >= 250) {   // 250 interrupts = 1 second
        secondCount = 0;        // Reset second counter
        oncePerSecond();        // Trigger brightness change
    }
}

/*
 * Master State Machine Controller
 * Advances all state machines on each timer interrupt
 */
void timeAdvStateMachines() 
{
    /*
     * Called every 1/250 seconds to update all state machines
     * This provides the main timing coordination for the system
     */
    blinkUpdate();              // Update LED PWM state
    secondUpdate();             // Update second timing state
}

/*
 * Watchdog Timer Interrupt Service Routine
 * Main timing source - executes 250 times per second
 */
void __interrupt_vec(WDT_VECTOR) WDT()
{
    /*
     * This interrupt provides the 4ms time base for all state machines
     * Frequency: 250 Hz (every 4 milliseconds)
     */
    timeAdvStateMachines();     // Advance all state machines
}
