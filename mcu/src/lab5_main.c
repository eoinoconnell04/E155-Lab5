/*
Author: Eoin O'Connell
Email: eoconnell@hmc.edu
Date: Oct. 6, 2025
File function: This program uses an algorithm to sense quadrature encoder pulses and convert these into motor velocity and direction.
*/

#include "main.h"

#define A_PIN PA6 
#define B_PIN PA9

volatile uint32_t last_time = 0;
volatile uint32_t current_time = 0; 
volatile int direction = 0;   // +1 or -1
volatile float velocity = 0;  // ticks per second

// Function Prototypes
void initTimer(void);
void configureInterrupts(void);
void updateVelocity(void);
int _write(int file, char *ptr, int len);

// Main Function
int main(void) {

    configureFlash();

    // Use 80 Mhz PLL
    configureClock();
    
    // Enable GPIO ports
    gpioEnable(GPIO_PORT_A);

    // Configure encoder pins as inputs with pull-ups
    pinMode(A_PIN, GPIO_INPUT);
    pinMode(B_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= (_VAL2FLD(GPIO_PUPDR_PUPD6, 0b01)); // PA6 pull-up
    GPIOA->PUPDR |= (_VAL2FLD(GPIO_PUPDR_PUPD9, 0b01)); // PA9 pull-up

    // Initialize delay timer for printing values
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;   
    initTIM(DELAY_TIM);

    // Initialize 32 bit timer for measuring time between pulses
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initCounterTIM(COUNT_TIM);

    configureInterrupts();

    // enable interrupts globally
    __enable_irq();

    while (1) {
        delay_millis(DELAY_TIM, 800);

        
        volatile uint32_t now = TIM2->CNT;
        //printf("Current Time: %d \n", now);
        if ((now - current_time) > 100000) { // if too long between interrupts then assume fully stopped
            velocity = 0;
        }
        if (direction == 1){
            printf("%.3f Hz CW\n", velocity);
        }
        else {
            printf("%.3f Hz CCW\n", velocity);
        }
    }
}

void configureInterrupts(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Configure EXTI lines for PA6 and PA9
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI6, 0b000); // Select PA6
    SYSCFG->EXTICR[2] |= _VAL2FLD(SYSCFG_EXTICR3_EXTI9, 0b000); // Select PA9

    // Unmask interrupts
    EXTI->IMR1 |= (1 << 6) | (1 << 9);

    // Trigger on both rising and falling edges for both pins
    EXTI->RTSR1 |= (1 << 6) | (1 << 9); 
    EXTI->FTSR1 |= (1 << 6) | (1 << 9);

    // Enable EXTI lines in NVIC
    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);
}

// Reads timer to update velocity
void updateVelocity(void) {
    last_time = current_time; // save previous value
    current_time = TIM2->CNT; // read current time

    // Compute velocity (ticks per second)
    velocity = 1000000.0f / (float)(current_time - last_time) / 408.0f / 2.0f;  //  timer is at 1 MHz, divide by # of ticks, PPR, and # of edges 

}

// Interrupt handler (same handler for both pin a6 and a9)
// Triggers: Rising and Falling Edges of Both pins a6 and pins a9
// Effects: changes the velocity and direction variables (velocity variabled changed through sub function updateVelocity)
void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR1 & (1 << 6)) {
        EXTI->PR1 |= (1 << 6); // clear pending
        
        updateVelocity();

        int a = digitalRead(A_PIN);
        int b = digitalRead(B_PIN);

        if (a == b)
            direction = -1;  // reverse
        else
            direction = +1;  // forward
    }

    if (EXTI->PR1 & (1 << 9)) {
        EXTI->PR1 |= (1 << 9); // clear pending

        // updateVelocity(); removing for smoother output

        int a = digitalRead(A_PIN);
        int b = digitalRead(B_PIN);

        if (a == b)
            direction = +1;  // forward
        else
            direction = -1;  // reverse
    }
}

// Function used by printf to send characters to the laptop (taken from E155 website)
int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}
