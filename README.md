### sendData() for STM32U073Cx 
_by Dominika Kubicz (25.02.2025)_

This solution consists of 3 supporting functions:
1. Configuring the PA9 pin for USART1_TX
2. Setting up USART1
3. Setting up DMA

The sendData() function sets up the 3 listed peripherals then waits for the DMA and UART transfers to complete and returns errors if they don't complete successfully. 
