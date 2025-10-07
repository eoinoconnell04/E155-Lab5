/*
Author: Eoin O'Connell
Email: eoconnell@hmc.edu
Date: Sep. 29, 2025
File function: C functions to go along with TIM struct
*/

#include "STM32L432KC_TIM15.h"

// Function configureTIM16:
// Sets up TIM15 for use for measuring duration of a note
// Uses a prescaler of 700
// No arguments
void configureTIM15(void) {
    
    TIM15->CR1 &= ~(1<<0); // disable counter
    TIM15->PSC = 700; // PSC = 700
    TIM15->EGR |= (1<<0); // trigger an update
    TIM15->CR1 |= (1<<0); // enable counter
}

// Function configureTIM16:
// Sets up TIM16 for use for setting the frequency of a note
// Uses a prescaler of 6
// Uses duty cycle of 50%
// No arguments
void configureTIM16(void){

    // Set PWM mode
    TIM16->CCMR1 &= ~(0b111 << 4);  // clear OC1M[2:0] 
    TIM16->CCMR1 |= (0b110 << 4);  // set PWM mode 1
    TIM16->CCMR1 |= (1 << 3);       // Set preload enable
    TIM16->CCMR1 &= ~(0b11 << 0);  // Set CC1 channel as output

    TIM16->CR1 |= (1 << 7); // enable ARPE
    TIM16->CCER |= (1 << 0); // Capture/Compare 1 output enable
    TIM16->BDTR |= (1 << 15); // Main output enable

    // set Prescaler
    TIM16->PSC = 6; // PSC = 6
    TIM16->ARR = 0xFFFF;  // auto-reload

    // Update changes and enable counter 
    TIM16->CR1 &= ~(1 << 0); // disable counter
    TIM16->EGR |= (1 << 0); // trigger an update
    TIM16->CR1 |= (1 << 0); // enable counter
}

// Function setFreq:
// Sets the frequency (in Hz) on TIM16
// Arguments: int freq specifies the frequency of a note in Hz
void setFreq(int freq){
    if (freq == 0){
        // If freq == 0 it is supposed to be silent
        TIM16->CCR1 = 0; // 0 Percent duty cycle 
    } else {
        // Toggle pin at freq
        TIM16->ARR = (80000000/(6+1)/freq) - 1; // 80 MHz / (PSC + 1) / Freq -1 yeields count to produce desiredd freq
        TIM16->CCR1 = ((80000000/(6+1)/freq) - 1)/2; // 50 Percent duty cycle 
        TIM16->EGR |= (1 << 0); // trigger an update
        TIM16->CR1 |= (1 << 0); // enable counter

    }
}

// Function setDur:
// Sets the delay (in ms) on TIM15
// Num cycles delay = (ARR+1)*(PSC+1)
// Arguments: int dur specifies the duration of a note in ms
void setDur(int dur){
    int cycles_per_ms = 80000000/1000/701; // 80 MHz / (1 s to 1000 ms)/ (Prescaler + 1) 

    TIM15->ARR = cycles_per_ms*dur; // Sets auto reload register to desired number of ticks
    TIM15->EGR |= (1 << 0); // trigger an update
    TIM15->SR &= ~(1 << 0); // clear UIF bit
    TIM15->CNT = 0; // reset counter before while loop
    
    while (!(TIM15->SR & (1<<0))); // waits until counter reaches tick count
}
