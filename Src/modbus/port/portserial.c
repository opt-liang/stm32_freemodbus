/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "usart.h"
extern UART_HandleTypeDef huart1;
/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

static void modbus_uart_config( UART_HandleTypeDef *huart, int baud ){

    UART_REINIT:
    HAL_UART_DeInit( huart );
    huart->Init.BaudRate = baud;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if( HAL_UART_Init(huart) != HAL_OK ){
        goto UART_REINIT;
    }

    if( huart->Instance == USART1 ){
        /* UART3_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);           //add code
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        USART1->SR;
    }else{
        printf("modbus_uart_config ERROR PARA HUART Non-existent\r\n");
    }
}

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
    
    if( xRxEnable ){
        __HAL_UART_ENABLE_IT( &huart1, UART_IT_RXNE );
    }else{
        __HAL_UART_DISABLE_IT( &huart1, UART_IT_RXNE );
    }
    
    if( xTxEnable ){
        __HAL_UART_ENABLE_IT( &huart1, UART_IT_TXE );
    }else{
        __HAL_UART_DISABLE_IT( &huart1, UART_IT_TXE );
    }
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    modbus_uart_config( &huart1, ulBaudRate );
    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
	return ( HAL_OK == HAL_UART_Transmit( &huart1, (uint8_t*)&ucByte, 1, 100 ) );
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
    *pucByte = (uint8_t)( huart1.Instance->DR & 0x000000FF );
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void modbus_uart_api( void ){
	if( (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE) != RESET) && (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) ) {
		prvvUARTRxISR();
	}
	if( (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE) != RESET) && (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) != RESET) ) {
		prvvUARTTxReadyISR();
	}
}










