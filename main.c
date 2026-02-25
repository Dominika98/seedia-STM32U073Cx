#pragma once
#include "includes/stm32u073xx.h"

int setup_tx_pin()
{
    // USART1 TX Pin is PA9
    // enable GPIOA clock 
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN_Msk;
    // set AF (0b10)
    GPIOA->MODER &= ~(GPIO_MODER_MODE9_1 | GPIO_MODER_MODE9_0);  // clean both bits
    GPIOA->MODER |= GPIO_MODER_MODE9_1;                          // set bit 1
    // setting AF7 - USART_TX
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL9_3 | GPIO_AFRH_AFSEL9_2 | GPIO_AFRH_AFSEL9_1 | GPIO_AFRH_AFSEL9_0);  // clean all bits
    GPIOA->AFR[1] |= (GPIO_AFRH_AFSEL9_2 | GPIO_AFRH_AFSEL9_1 | GPIO_AFRH_AFSEL9_0);                        // set 0b0111

    return 0;
}

int setup_USART1()
{
    // enable usart clock
    RCC->APBENR2 |= RCC_APBENR2_USART1EN_Msk;
    // make sure usart disabled
    USART1->CR1 &= ~USART_CR1_UE_Msk;
    // set baud rate (assuming fck = 16MHz)
    USART1->BRR = 0x0683;
    // set word length 8bit
    USART1->CR1 &= ~(USART_CR1_M0_Msk | USART_CR1_M1_Msk);
    // partity control disabled
    USART1->CR1 &= ~USART_CR1_PCE_Msk;
    // oversampling by 16 
    USART1->CR1 &= ~USART_CR1_OVER8_Msk;
    // setting 1 stop bit
    USART1->CR2 &= ~USART_CR2_STOP_Msk;
    // Enable transmitter
    USART1->CR1 |= USART_CR1_TE_Msk;
    // enable DMA transmitter
    USART1->CR3 |= USART_CR3_DMAT_Msk;
    // enable usart
    USART1->CR1 |= USART_CR1_UE_Msk;
    
    return 0;
}

int setup_DMA(uint8_t* buffer, uint8_t data_len)
{
    // enable clock
    RCC->AHBENR |= RCC_AHBENR_DMA1EN_Msk;
    // disable DMA
    DMA1_Channel1->CCR &= ~DMA_CCR_EN_Msk;
    // wait for it to be disabled
    uint32_t start = systick_ms;
    while (DMA1_Channel1->CCR & DMA_CCR_EN_Msk)
    {
        if ((systick_ms - start) > 500) // timeout 500 ms 
        { 
            return -1; 
        }
    }
    // clean flags
    DMA1->IFCR = DMA_IFCR_CTCIF1 | DMA_IFCR_CGIF1 | DMA_IFCR_CTEIF1;
    // setting peripheral address register
    DMA1_Channel1->CPAR = (uint32_t)&USART1->TDR;
    // setting memory address register
    DMA1_Channel1->CMAR = (uint32_t)buffer;
    // clean
    DMA1_Channel1->CCR = 0;
    // setting memory increment mode, direction, priority high
    DMA1_Channel1->CCR = DMA_CCR_MINC_Msk | DMA_CCR_DIR_Msk  | DMA_CCR_PL_1;
    // setting number of data to transfer
    DMA1_Channel1->CNDTR = data_len;
    // setting DMA request identification (USART1_TX)
    DMAMUX1_Channel1->CCR = 70;

    return 0;
}

int sendData(uint8_t data_len) 
{
    uint8_t buffer_tx[data_len] = {};

    // zapełnianie buffora
    for(int i = 0; i < data_len; i++)
    {
        buffer_tx[i] = i+2;
    }

    if(!(setup_tx_pin() == 0))
    {
        return -1;
    }

    if(!(setup_USART1() == 0))
    {
        return -1;
    }
    
    if(!(setup_DMA(buffer_tx, data_len) == 0))
    {
        return -1;
    }

    // start the DMA
    DMA1_Channel1->CCR |= DMA_CCR_EN_Msk;

    uint32_t start = systick_ms;
    // wait for the DMA transfer
    while((DMA1_Channel1->CNDTR & DMA_CNDTR_NDT_Msk) != 0)
    {
        if ((systick_ms - start) > 500) // timeout 500 ms 
        { 
            return -1; 
        }
    }

    start = systick_ms;
    while(!(USART1->ISR & USART_ISR_TC_Msk))
    {
        if ((systick_ms - start) > 500) // timeout 500 ms 
        { 
            return -1; 
        }
    }

    return 0;
}