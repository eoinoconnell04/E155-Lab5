/*
Author: Eoin O'Connell
Email: eoconnell@hmc.edu
Date: Sep. 29, 2025
File function: Header file containing the TIM struct
*/

#include <stdint.h>

// Base addresses
#define TIM15_BASE (0x40014000UL) // base address of TIM15
#define TIM16_BASE (0x40014400UL) // base address of TIM16

typedef struct {
    volatile uint32_t CR1; // offset 0x00: control register 1
    volatile uint32_t CR2; // offset 0x04: control register 2
    volatile uint32_t SMCR; // offset 0x08: slave mode control register
    volatile uint32_t DIER; // offset 0x0C: DMA/interrupt enable register
    volatile uint32_t SR; // offset 0x10: register status register
    volatile uint32_t EGR; // offset 0x14: event generation register
    volatile uint32_t CCMR1; // offset 0x18: capture/compare mode register 1
    volatile uint32_t reserved1;
    volatile uint32_t CCER; // offset 0x20: capture/compare enable 
    volatile uint32_t CNT; // offset 0x24: counter
    volatile uint32_t PSC; // offset 0x28: prescaler
    volatile uint32_t ARR; // offset 0x2C: auto-reload register
    volatile uint32_t RCR; // offset 0x30: repetition counter register
    volatile uint32_t CCR1; // offset 0x34: capture/compare register 1 
    volatile uint32_t CCR2; // offset 0x38: capture/compare register 2
    volatile uint32_t reserved2;
    volatile uint32_t reserved3;
    volatile uint32_t BDTR; // offset 0x44: break and dead-time register
    volatile uint32_t DCR; // offset 0x48: DMA control register
    volatile uint32_t DMAR; // offset 0x4C: address for full transfer
    volatile uint32_t OR1; // offset 0x50: option register 1
    volatile uint32_t reserved4; // 0x54
    volatile uint32_t reserved5; // 0x58
    volatile uint32_t reserved6; // 0x5C
    volatile uint32_t OR2; // offset 0x60: option register 2

} TIMxx_TypeDef;

#define TIM15 ((TIMxx_TypeDef *) TIM15_BASE)
#define TIM16 ((TIMxx_TypeDef *) TIM16_BASE)

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

void configureTIM15(void);
void configureTIM16(void);
void setFreq(int freq); // sets the frequency on TIM16
void setDur(int dur);   // sets the wait delay (in ms) using TIM15

