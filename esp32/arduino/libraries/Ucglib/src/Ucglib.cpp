/*

  Ucglib.cpp

  ucglib = universal color graphics library
  ucglib = micro controller graphics library
  
  Universal uC Color Graphics Library
  
  Copyright (c) 2014, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of s_t ource code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  
*/

#include <SPI.h>
#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif
#include "Ucglib.h"

/*=========================================================================*/
/* 8 Bit SW SPI */

#if  defined(__SAM3X8E__)
//#elif defined(__SAM3X8E__)

//#define setbit(pio, mask) PIO_Set( (pio), (mask) )
//#define clrbit(pio, mask) PIO_Clear( (pio), (mask) )

#define setbit(pio, mask) ((pio)->PIO_SODR = (mask))
#define clrbit(pio, mask) ((pio)->PIO_CODR = (mask))

static void ucg_nano_delay(void)
{
  volatile uint32_t i;
  for( i = 0; i < 1; i++ )
  {
    __NOP;
  }
  //delayMicroseconds(1);
}

static void ucg_com_arduino_send_generic_SW_SPI(ucg_t *ucg, uint8_t data)
{
  uint32_t sda_pin = ucg->pin_list[UCG_PIN_SDA];
  uint32_t scl_pin = ucg->pin_list[UCG_PIN_SCL];
  Pio *sda_port = g_APinDescription[sda_pin].pPort;
  Pio *scl_port = g_APinDescription[scl_pin].pPort;
  uint8_t i = 8;
  sda_pin = g_APinDescription[sda_pin].ulPin;
  scl_pin = g_APinDescription[scl_pin].ulPin;

  do
  {
    if ( data & 128 )
    {
      setbit( sda_port, sda_pin) ;
    }
    else
    {
      clrbit( sda_port, sda_pin) ;
    }
    //delayMicroseconds(1);
    ucg_nano_delay();
    setbit( scl_port, scl_pin);
    //delayMicroseconds(1);
    ucg_nano_delay();
    i--;
    clrbit( scl_port, scl_pin) ;
    data <<= 1;
  } while( i > 0 );
  
}

#elif defined(__AVR__)

uint8_t u8g_bitData, u8g_bitNotData;
uint8_t u8g_bitClock, u8g_bitNotClock;
volatile uint8_t *u8g_outData;
volatile uint8_t *u8g_outClock;

static void ucg_com_arduino_init_shift_out(uint8_t dataPin, uint8_t clockPin)
{
  u8g_outData = portOutputRegister(digitalPinToPort(dataPin));
  u8g_outClock = portOutputRegister(digitalPinToPort(clockPin));
  u8g_bitData = digitalPinToBitMask(dataPin);
  u8g_bitClock = digitalPinToBitMask(clockPin);

  u8g_bitNotClock = u8g_bitClock;
  u8g_bitNotClock ^= 0x0ff;

  u8g_bitNotData = u8g_bitData;
  u8g_bitNotData ^= 0x0ff;
}


static void ucg_com_arduino_send_generic_SW_SPI(ucg_t *ucg, uint8_t val) UCG_NOINLINE;
static void ucg_com_arduino_send_generic_SW_SPI(ucg_t *ucg, uint8_t val)
{
  uint8_t cnt = 8;
  uint8_t bitData = u8g_bitData;
  uint8_t bitNotData = u8g_bitNotData;
  uint8_t bitClock = u8g_bitClock;
  uint8_t bitNotClock = u8g_bitNotClock;
  volatile uint8_t *outData = u8g_outData;
  volatile uint8_t *outClock = u8g_outClock;
  
  UCG_ATOMIC_START();
  do
  {
    if ( val & 128 )
      *outData |= bitData;
    else
      *outData &= bitNotData;
   
    *outClock |= bitClock;
    val <<= 1;
    cnt--;
    *outClock &= bitNotClock;
  } while( cnt != 0 );
  UCG_ATOMIC_END();
  
}

#else

static void ucg_com_arduino_send_generic_SW_SPI(ucg_t *ucg, uint8_t data)
{
  uint8_t i = 8;
  
  do
  {
    if ( data & 128 )
    {
      digitalWrite(ucg->pin_list[UCG_PIN_SDA], 1 );
    }
    else
    {
      digitalWrite(ucg->pin_list[UCG_PIN_SDA], 0 );
    }
    // no delay required, also Arduino Due is slow enough
    //delayMicroseconds(1);
    digitalWrite(ucg->pin_list[UCG_PIN_SCL], 1 );
    //delayMicroseconds(1);
    i--;
    digitalWrite(ucg->pin_list[UCG_PIN_SCL], 0 );
    //delayMicroseconds(1);
    data <<= 1;
  } while( i > 0 );
  
}

#endif

static int16_t ucg_com_arduino_generic_SW_SPI(ucg_t *ucg, int16_t msg, uint16_t arg, uint8_t *data)
{

  switch(msg)
  {
    case UCG_COM_MSG_POWER_UP:
      /* "data" is a pointer to ucg_com_info_t structure with the following information: */
      /*	((ucg_com_info_t *)data)->serial_clk_speed value in nanoseconds */
      /*	((ucg_com_info_t *)data)->parallel_clk_speed value in nanoseconds */

#ifdef __AVR__
      ucg_com_arduino_init_shift_out(ucg->pin_list[UCG_PIN_SDA], ucg->pin_list[UCG_PIN_SCL]);
#endif
    
      /* setup pins */
      pinMode(ucg->pin_list[UCG_PIN_CD], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_SDA], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_SCL], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_CS], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_RST], OUTPUT);

      digitalWrite(ucg->pin_list[UCG_PIN_CD], 1);
      digitalWrite(ucg->pin_list[UCG_PIN_SDA], 1);
      digitalWrite(ucg->pin_list[UCG_PIN_SCL], 0);
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], 1);

      break;
    case UCG_COM_MSG_POWER_DOWN:
      break;
    case UCG_COM_MSG_DELAY:
      delayMicroseconds(arg);
      break;
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], arg);
      break;
    case UCG_COM_MSG_CHANGE_CS_LINE:
#ifdef __AVR__
      ucg_com_arduino_init_shift_out(ucg->pin_list[UCG_PIN_SDA], ucg->pin_list[UCG_PIN_SCL]);
#endif    
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], arg);      
      break;
    case UCG_COM_MSG_CHANGE_CD_LINE:
      digitalWrite(ucg->pin_list[UCG_PIN_CD], arg);
      break;
    case UCG_COM_MSG_SEND_BYTE:
      ucg_com_arduino_send_generic_SW_SPI(ucg, arg);
      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      while( arg > 0 ) {
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[0]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[0]);
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[1]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[0]);
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[1]);
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[2]);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_STR:
      while( arg > 0 ) {
	ucg_com_arduino_send_generic_SW_SPI(ucg, *data++);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      while(arg > 0)
      {
	if ( *data != 0 )
	{
	  if ( *data == 1 )
	  {
	    digitalWrite(ucg->pin_list[UCG_PIN_CD], 0);
	  }
	  else
	  {
	    digitalWrite(ucg->pin_list[UCG_PIN_CD], 1);
	  }
	}
	data++;
	ucg_com_arduino_send_generic_SW_SPI(ucg, *data);
	data++;
	arg--;
      }
      break;
  }
  return 1;
}

void Ucglib4WireSWSPI::begin(uint8_t is_transparent)
{ 
  ucg_Init(&ucg, dev_cb, ext_cb, ucg_com_arduino_generic_SW_SPI); 
  ucg_SetFontMode(&ucg, is_transparent);
}


/*=========================================================================*/
/* 8 Bit SW SPI for ILI9325 (mode IM3=0, IM2=1, IM1=0, IM0=0 */

static int16_t ucg_com_arduino_illi9325_SW_SPI(ucg_t *ucg, int16_t msg, uint16_t arg, uint8_t *data)
{

  switch(msg)
  {
    case UCG_COM_MSG_POWER_UP:
      /* "data" is a pointer to ucg_com_info_t structure with the following information: */
      /*	((ucg_com_info_t *)data)->serial_clk_speed value in nanoseconds */
      /*	((ucg_com_info_t *)data)->parallel_clk_speed value in nanoseconds */

#ifdef __AVR__
      ucg_com_arduino_init_shift_out(ucg->pin_list[UCG_PIN_SDA], ucg->pin_list[UCG_PIN_SCL]);
#endif
    
      /* setup pins */
      pinMode(ucg->pin_list[UCG_PIN_CD], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_SDA], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_SCL], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_CS], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_RST], OUTPUT);

      digitalWrite(ucg->pin_list[UCG_PIN_CD], 1);
      digitalWrite(ucg->pin_list[UCG_PIN_SDA], 1);
      digitalWrite(ucg->pin_list[UCG_PIN_SCL], 0);
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], 1);

      break;
    case UCG_COM_MSG_POWER_DOWN:
      break;
    case UCG_COM_MSG_DELAY:
      delayMicroseconds(arg);
      break;
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], arg);
      break;
      
    case UCG_COM_MSG_CHANGE_CS_LINE:
#ifdef __AVR__
      ucg_com_arduino_init_shift_out(ucg->pin_list[UCG_PIN_SDA], ucg->pin_list[UCG_PIN_SCL]);
#endif    
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], arg);      
      break;
      
    case UCG_COM_MSG_CHANGE_CD_LINE:
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
      {
	digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);      
	digitalWrite(ucg->pin_list[UCG_PIN_CS], 0);      
      }

      if ( ucg->com_status & UCG_COM_STATUS_MASK_CD )
	ucg_com_arduino_send_generic_SW_SPI(ucg, 0x072);
      else
	ucg_com_arduino_send_generic_SW_SPI(ucg, 0x070);      
	
      break;
      
    case UCG_COM_MSG_SEND_BYTE:
      ucg_com_arduino_send_generic_SW_SPI(ucg, arg);
      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      while( arg > 0 ) {
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[0]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[0]);
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[1]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[0]);
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[1]);
	ucg_com_arduino_send_generic_SW_SPI(ucg, data[2]);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_STR:
      while( arg > 0 ) {
	ucg_com_arduino_send_generic_SW_SPI(ucg, *data++);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      while(arg > 0)
      {
	if ( *data != 0 )
	{
	  if ( *data == 1 )
	  {
	    if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	    {
	      digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);      
	      digitalWrite(ucg->pin_list[UCG_PIN_CS], 0);      
	    }
	    ucg_com_arduino_send_generic_SW_SPI(ucg, 0x070);
	  }
	  else
	  {
	    if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	    {
	      digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);      
	      digitalWrite(ucg->pin_list[UCG_PIN_CS], 0);      
	    }
	    ucg_com_arduino_send_generic_SW_SPI(ucg, 0x072);
	  }
	}
	data++;
	ucg_com_arduino_send_generic_SW_SPI(ucg, *data);
	data++;
	arg--;
      }
      break;
  }
  return 1;
}

void Ucglib3WireILI9325SWSPI::begin(uint8_t is_transparent)
{ 
  ucg_Init(&ucg, dev_cb, ext_cb, ucg_com_arduino_illi9325_SW_SPI); 
  ucg_SetFontMode(&ucg, is_transparent);
}


/*=========================================================================*/
/* 9 Bit SW SPI */

static void ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg_t *ucg, uint8_t first_bit, uint8_t data)
{
  uint8_t i;

  if ( first_bit != 0 )
  {
    digitalWrite(ucg->pin_list[UCG_PIN_SDA], 1 );
  }
  else
  {
    digitalWrite(ucg->pin_list[UCG_PIN_SDA], 0 );
  }
  // no delay required, also Arduino Due is slow enough
  //delayMicroseconds(1);
  digitalWrite(ucg->pin_list[UCG_PIN_SCL], 1 );
  //delayMicroseconds(1);
  digitalWrite(ucg->pin_list[UCG_PIN_SCL], 0 );
  //delayMicroseconds(1);

  i = 8;
  do
  {
    if ( data & 128 )
    {
      digitalWrite(ucg->pin_list[UCG_PIN_SDA], 1 );
    }
    else
    {
      digitalWrite(ucg->pin_list[UCG_PIN_SDA], 0 );
    }
    // no delay required, also Arduino Due is slow enough
    //delayMicroseconds(1);
    digitalWrite(ucg->pin_list[UCG_PIN_SCL], 1 );
    //delayMicroseconds(1);
    i--;
    digitalWrite(ucg->pin_list[UCG_PIN_SCL], 0 );
    //delayMicroseconds(1);
    data <<= 1;
  } while( i > 0 );
  
}

static int16_t ucg_com_arduino_3wire_9bit_SW_SPI(ucg_t *ucg, int16_t msg, uint16_t arg, uint8_t *data)
{

  switch(msg)
  {
    case UCG_COM_MSG_POWER_UP:
      /* "data" is a pointer to ucg_com_info_t structure with the following information: */
      /*	((ucg_com_info_t *)data)->serial_clk_speed value in nanoseconds */
      /*	((ucg_com_info_t *)data)->parallel_clk_speed value in nanoseconds */
      
      /* setup pins */
      pinMode(ucg->pin_list[UCG_PIN_SDA], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_SCL], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_CS], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_RST], OUTPUT);

      digitalWrite(ucg->pin_list[UCG_PIN_SDA], 1);
      digitalWrite(ucg->pin_list[UCG_PIN_SCL], 0);
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], 1);

      break;
    case UCG_COM_MSG_POWER_DOWN:
      break;
    case UCG_COM_MSG_DELAY:
      delayMicroseconds(arg);
      break;
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], arg);
      break;
    case UCG_COM_MSG_CHANGE_CS_LINE:
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], arg);
      break;
    case UCG_COM_MSG_CHANGE_CD_LINE:
      /* ignored, there is not CD line */
      break;
    case UCG_COM_MSG_SEND_BYTE:
      ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, arg);
      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      while( arg > 0 ) {
	ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[0]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[0]);
	ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[1]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[0]);
	ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[1]);
	ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[2]);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_STR:
      while( arg > 0 ) {
	ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, *data++);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      {
	uint8_t last_cd = ucg->com_status &UCG_COM_STATUS_MASK_CD;
	while(arg > 0)
	{
	  if ( *data != 0 )
	  {
	    if ( *data == 1 )
	    {
	      last_cd = 0;
	    }
	    else
	    {
	      last_cd = 1;
	    }
	  }
	  data++;
	  ucg_com_arduino_send_3wire_9bit_SW_SPI(ucg, last_cd, *data); 
	  data++;
	  arg--;
	}
      }
      break;
  }
  return 1;
}

void Ucglib3Wire9bitSWSPI::begin(uint8_t is_transparent)
{ 
  ucg_Init(&ucg, dev_cb, ext_cb, ucg_com_arduino_3wire_9bit_SW_SPI); 
  ucg_SetFontMode(&ucg, is_transparent);
}

/*=========================================================================*/
/* 9 Bit HW SPI */

#define UCG_COM_ARDUINO_3WIRE_8BIT_BUF_LEN 9
static uint8_t ucg_com_3wire_9bit_buffer[UCG_COM_ARDUINO_3WIRE_8BIT_BUF_LEN];
static uint8_t ucg_com_3wire_9bit_buf_bytepos;
static uint8_t ucg_com_3wire_9bit_buf_bitpos;
static uint8_t ucg_com_3wire_9bit_cd_mask;

static void ucg_com_arduino_init_3wire_9bit_HW_SPI(ucg_t *ucg) UCG_NOINLINE;
static void ucg_com_arduino_init_3wire_9bit_HW_SPI(ucg_t *ucg)
{
  uint8_t i;
  ucg_com_3wire_9bit_buf_bytepos = 0;
  ucg_com_3wire_9bit_buf_bitpos = 7;
  ucg_com_3wire_9bit_cd_mask = 128;
  for( i = 0; i < UCG_COM_ARDUINO_3WIRE_8BIT_BUF_LEN; i++ )
    ucg_com_3wire_9bit_buffer[i] = 0; /* this is also the NOP command for the PCF8833 */
}

static void ucg_com_arduino_flush_3wire_9bit_HW_SPI(ucg_t *ucg) UCG_NOINLINE;
static void ucg_com_arduino_flush_3wire_9bit_HW_SPI(ucg_t *ucg)
{
  uint8_t i;
  if ( ucg_com_3wire_9bit_buf_bytepos == 0 && ucg_com_3wire_9bit_buf_bitpos == 7 )
    return;
  
  for( i = 0; i < UCG_COM_ARDUINO_3WIRE_8BIT_BUF_LEN; i++ )
    SPI.transfer(ucg_com_3wire_9bit_buffer[i] );
  
  ucg_com_arduino_init_3wire_9bit_HW_SPI(ucg);
}

static void ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg_t *ucg, uint8_t first_bit, uint8_t data)
{
  
  if ( first_bit != 0 )
    ucg_com_3wire_9bit_buffer[ucg_com_3wire_9bit_buf_bytepos] |= ucg_com_3wire_9bit_cd_mask;
  
  if ( ucg_com_3wire_9bit_buf_bitpos > 0 )
  {
    ucg_com_3wire_9bit_buf_bitpos--;
    ucg_com_3wire_9bit_cd_mask >>= 1;
  }
  else
  {
    ucg_com_3wire_9bit_buf_bitpos = 7;
    ucg_com_3wire_9bit_buf_bytepos++;
    ucg_com_3wire_9bit_cd_mask = 128;
  }
  
  ucg_com_3wire_9bit_buffer[ucg_com_3wire_9bit_buf_bytepos] |=  data >> (7-ucg_com_3wire_9bit_buf_bitpos);

  if ( ucg_com_3wire_9bit_buf_bitpos == 7 )
  {
    ucg_com_3wire_9bit_buf_bytepos++;
    if ( ucg_com_3wire_9bit_buf_bytepos >= UCG_COM_ARDUINO_3WIRE_8BIT_BUF_LEN )
      ucg_com_arduino_flush_3wire_9bit_HW_SPI(ucg);      
  }
  else
  {
    ucg_com_3wire_9bit_buf_bytepos++;
    ucg_com_3wire_9bit_buffer[ucg_com_3wire_9bit_buf_bytepos] |=  data << (ucg_com_3wire_9bit_buf_bitpos+1);
  }
}

static int16_t ucg_com_arduino_3wire_9bit_HW_SPI(ucg_t *ucg, int16_t msg, uint16_t arg, uint8_t *data)
{

  switch(msg)
  {
    case UCG_COM_MSG_POWER_UP:
      /* "data" is a pointer to ucg_com_info_t structure with the following information: */
      /*	((ucg_com_info_t *)data)->serial_clk_speed value in nanoseconds */
      /*	((ucg_com_info_t *)data)->parallel_clk_speed value in nanoseconds */
      
      ucg_com_arduino_init_3wire_9bit_HW_SPI(ucg);
    
      /* setup pins */
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_CS], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_RST], OUTPUT);

      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], 1);

      /* setup Arduino SPI */
      SPI.begin();
#if defined(__AVR__)
      SPI.setClockDivider( SPI_CLOCK_DIV2 );
      //SPI.setClockDivider( SPI_CLOCK_DIV64  );
      //SPI.setDataMode(SPI_MODE0);
    
#endif
#if defined(__SAM3X8E__)
      SPI.setClockDivider( (((ucg_com_info_t *)data)->serial_clk_speed * 84L + 999)/1000L );
#endif
      SPI.setDataMode(SPI_MODE0);
      SPI.setBitOrder(MSBFIRST);
      break;
    case UCG_COM_MSG_POWER_DOWN:
      SPI.end();
      break;
    case UCG_COM_MSG_DELAY:
      /* flush pending data first, then do the delay */
      ucg_com_arduino_flush_3wire_9bit_HW_SPI(ucg);      
      delayMicroseconds(arg);
      break;
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], arg);
      break;
    case UCG_COM_MSG_CHANGE_CS_LINE:
      if ( arg != 0 )
	ucg_com_arduino_flush_3wire_9bit_HW_SPI(ucg);      
      
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], arg);
      
      if ( arg == 0 )
	ucg_com_arduino_init_3wire_9bit_HW_SPI(ucg);
      
      break;
    case UCG_COM_MSG_CHANGE_CD_LINE:
      /* not used */
      break;
    case UCG_COM_MSG_SEND_BYTE:
      ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, arg);
      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      while( arg > 0 ) {
	ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[0]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[0]);
	ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[1]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[0]);
	ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[1]);
	ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, data[2]);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_STR:
      while( arg > 0 ) {
	ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, ucg->com_status &UCG_COM_STATUS_MASK_CD, *data++);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      {
	uint8_t last_cd = ucg->com_status &UCG_COM_STATUS_MASK_CD;
	while(arg > 0)
	{
	  if ( *data != 0 )
	  {
	    if ( *data == 1 )
	    {
	      last_cd = 0;
	    }
	    else
	    {
	      last_cd = 1;
	    }
	  }
	  data++;
	  ucg_com_arduino_send_3wire_9bit_HW_SPI(ucg, last_cd, *data); 
	  data++;
	  arg--;
	}
      }
      break;
  }
  return 1;
}

void Ucglib3Wire9bitHWSPI::begin(uint8_t is_transparent)
{ 
  ucg_Init(&ucg, dev_cb, ext_cb, ucg_com_arduino_3wire_9bit_HW_SPI); 
  ucg_SetFontMode(&ucg, is_transparent);
}


/*=========================================================================*/
/* 8 Bit Parallel */


#if defined(__PIC32MX) || defined(__arm__)
/* CHIPKIT PIC32 */
static volatile uint32_t *u8g_data_port[9];
static uint32_t u8g_data_mask[9];
#else
static volatile uint8_t *u8g_data_port[9];
static uint8_t u8g_data_mask[9];
#endif

static void ucg_com_arduino_init_8bit(ucg_t *ucg)
{
  #if defined(ESP_PLATFORM)
  u8g_data_port[0] =  0;
  u8g_data_port[1] =  0;
  u8g_data_port[2] =  0;
  u8g_data_port[3] =  0;
  u8g_data_port[4] =  0;
  u8g_data_port[5] =  0;
  u8g_data_port[6] =  0;
  if ( ucg->pin_list[UCG_PIN_D6] != UCG_PIN_VAL_NONE )
  {
    u8g_data_port[6] =  0;
  }
  
  if ( ucg->pin_list[UCG_PIN_D7] != UCG_PIN_VAL_NONE )
  {
    u8g_data_port[7] =  0;
  }  

  u8g_data_port[8] =  0;
  #else
  u8g_data_port[0] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_D0]));
  u8g_data_port[1] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_D1]));
  u8g_data_port[2] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_D2]));
  u8g_data_port[3] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_D3]));
  u8g_data_port[4] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_D4]));
  u8g_data_port[5] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_D5]));
  if ( ucg->pin_list[UCG_PIN_D6] != UCG_PIN_VAL_NONE )
  {
    u8g_data_port[6] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_D6]));
  }
  
  if ( ucg->pin_list[UCG_PIN_D7] != UCG_PIN_VAL_NONE )
  {
    u8g_data_port[7] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_D7]));
  }  

  u8g_data_port[8] =  portOutputRegister(digitalPinToPort(ucg->pin_list[UCG_PIN_WR]));
  #endif
  u8g_data_mask[0] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_D0]);  
  u8g_data_mask[1] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_D1]);  
  u8g_data_mask[2] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_D2]);  
  u8g_data_mask[3] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_D3]);  
  u8g_data_mask[4] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_D4]);  
  u8g_data_mask[5] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_D5]);  
  
  if ( ucg->pin_list[UCG_PIN_D6] != UCG_PIN_VAL_NONE )
  {
    u8g_data_mask[6] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_D6]);  
  }
  
  if ( ucg->pin_list[UCG_PIN_D7] != UCG_PIN_VAL_NONE )
  {
    u8g_data_mask[7] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_D7]);  
  }  
  u8g_data_mask[8] =  digitalPinToBitMask(ucg->pin_list[UCG_PIN_WR]);  
  
}

static void ucg_com_arduino_send_8bit(ucg_t *ucg, uint8_t data)
{
  int i;
  #if defined(__arm__)
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
#endif
  for( i = 0; i < 8; i++ )
  {
    if ( data & 1 )
      *u8g_data_port[i] |= u8g_data_mask[i]; 
    else
      *u8g_data_port[i] &= ~u8g_data_mask[i]; 
    data >>= 1;
  }

  #if defined(__arm__)
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  delayMicroseconds(1);
#elif defined(__AVR__)
#else
  delayMicroseconds(1);
#endif
  
  *u8g_data_port[8] &= ~u8g_data_mask[8]; 
  
#if defined(__arm__)
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  __NOP;
  delayMicroseconds(1);
#elif defined(__AVR__)
#else
  delayMicroseconds(1);
#endif
  
  *u8g_data_port[8] |= u8g_data_mask[8]; 
}

/*
static void ucg_com_arduino_send_8bit(ucg_t *ucg, uint8_t data)
{
    digitalWrite(ucg->pin_list[UCG_PIN_D0], (data & 1) == 0 ? 0 : 1 );
    digitalWrite(ucg->pin_list[UCG_PIN_D1], (data & 2) == 0 ? 0 : 1 );
    digitalWrite(ucg->pin_list[UCG_PIN_D2], (data & 4) == 0 ? 0 : 1 );
    digitalWrite(ucg->pin_list[UCG_PIN_D3], (data & 8) == 0 ? 0 : 1 );
    digitalWrite(ucg->pin_list[UCG_PIN_D4], (data & 16) == 0 ? 0 : 1 );
    digitalWrite(ucg->pin_list[UCG_PIN_D5], (data & 32) == 0 ? 0 : 1 );
    if ( ucg->pin_list[UCG_PIN_D6] != UCG_PIN_VAL_NONE )
      digitalWrite(ucg->pin_list[UCG_PIN_D6], (data & 64) == 0 ? 0 : 1 );
    if ( ucg->pin_list[UCG_PIN_D7] != UCG_PIN_VAL_NONE )
      digitalWrite(ucg->pin_list[UCG_PIN_D7], (data & 128) == 0 ? 0 : 1 );  
    delayMicroseconds(1);
    digitalWrite(ucg->pin_list[UCG_PIN_WR], 0);
    delayMicroseconds(1);
    digitalWrite(ucg->pin_list[UCG_PIN_WR], 1);
}
*/

static int16_t ucg_com_arduino_generic_8bit(ucg_t *ucg, int16_t msg, uint16_t arg, uint8_t *data)
{
  switch(msg)
  {
    case UCG_COM_MSG_POWER_UP:
      /* "data" is a pointer to ucg_com_info_t structure with the following information: */
      /*	((ucg_com_info_t *)data)->serial_clk_speed value in nanoseconds */
      /*	((ucg_com_info_t *)data)->parallel_clk_speed value in nanoseconds */
      
      /* setup pins */
      pinMode(ucg->pin_list[UCG_PIN_CD], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_WR], OUTPUT);
    
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_CS], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_RST], OUTPUT);

      pinMode(ucg->pin_list[UCG_PIN_D0], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_D1], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_D2], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_D3], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_D4], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_D5], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_D6] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_D6], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_D7] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_D7], OUTPUT);

      digitalWrite(ucg->pin_list[UCG_PIN_CD], 1);
      digitalWrite(ucg->pin_list[UCG_PIN_WR], 1);
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], 1);

      ucg_com_arduino_init_8bit(ucg);
      
      break;
    case UCG_COM_MSG_POWER_DOWN:
      break;
    case UCG_COM_MSG_DELAY:
      delayMicroseconds(arg);
      break;
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], arg);
      break;
    case UCG_COM_MSG_CHANGE_CS_LINE:
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], arg);
      break;
    case UCG_COM_MSG_CHANGE_CD_LINE:
      digitalWrite(ucg->pin_list[UCG_PIN_CD], arg);
      break;
    case UCG_COM_MSG_SEND_BYTE:
      ucg_com_arduino_send_8bit(ucg, arg);
      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      while( arg > 0 ) {
	ucg_com_arduino_send_8bit(ucg, data[0]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_8bit(ucg, data[0]);
	ucg_com_arduino_send_8bit(ucg, data[1]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_send_8bit(ucg, data[0]);
	ucg_com_arduino_send_8bit(ucg, data[1]);
	ucg_com_arduino_send_8bit(ucg, data[2]);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_STR:
      while( arg > 0 ) {
	ucg_com_arduino_send_8bit(ucg, *data++);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      while(arg > 0)
      {
	if ( *data != 0 )
	{
	  if ( *data == 1 )
	  {
	    digitalWrite(ucg->pin_list[UCG_PIN_CD], 0);
	  }
	  else
	  {
	    digitalWrite(ucg->pin_list[UCG_PIN_CD], 1);
	  }
	}
	data++;
	ucg_com_arduino_send_8bit(ucg, *data);
	data++;
	arg--;
      }
      break;
  }
  return 1;
}

void Ucglib8Bit::begin(uint8_t is_transparent)
{ 
  ucg_Init(&ucg, dev_cb, ext_cb, ucg_com_arduino_generic_8bit); 
  ucg_SetFontMode(&ucg, is_transparent);
}


/*=========================================================================*/
/* 8 Bit Parallel on Port D of AVR controller */

#ifdef __AVR__

static void ucg_com_arduino_port_d_send(uint8_t data, volatile uint8_t *port, uint8_t and_mask, uint8_t or_mask)
{
    PORTD = data;
    *port &= and_mask;
    *port |= or_mask;
}

static int16_t ucg_com_arduino_port_d(ucg_t *ucg, int16_t msg, uint16_t arg, uint8_t *data)
{
  switch(msg)
  {
    case UCG_COM_MSG_POWER_UP:
      /* "data" is a pointer to ucg_com_info_t structure with the following information: */
      /*	((ucg_com_info_t *)data)->serial_clk_speed value in nanoseconds */
      /*	((ucg_com_info_t *)data)->parallel_clk_speed value in nanoseconds */
      
      /* setup pins */
      pinMode(ucg->pin_list[UCG_PIN_CD], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_WR], OUTPUT);
      
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_CS], OUTPUT);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_RST], OUTPUT);

      pinMode(0, OUTPUT);
      pinMode(1, OUTPUT);
      pinMode(2, OUTPUT);
      pinMode(3, OUTPUT);
      pinMode(4, OUTPUT);
      pinMode(5, OUTPUT);
      pinMode(6, OUTPUT);
      pinMode(7, OUTPUT);

      digitalWrite(ucg->pin_list[UCG_PIN_CD], 1);
      digitalWrite(ucg->pin_list[UCG_PIN_WR], 1);
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], 1);
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], 1);

      break;
    case UCG_COM_MSG_POWER_DOWN:
      break;
    case UCG_COM_MSG_DELAY:
      delayMicroseconds(arg);
      break;
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], arg);
      break;
    case UCG_COM_MSG_CHANGE_CS_LINE:
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], arg);
      break;
    case UCG_COM_MSG_CHANGE_CD_LINE:
      if ( arg == 0 )
	*ucg->data_port[UCG_PIN_CD] &= ~ucg->data_mask[UCG_PIN_CD];
      else
	*ucg->data_port[UCG_PIN_CD] |= ucg->data_mask[UCG_PIN_CD];
      //digitalWrite(ucg->pin_list[UCG_PIN_CD], arg);
      break;
    case UCG_COM_MSG_SEND_BYTE:
      ucg_com_arduino_port_d_send(arg, ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);

      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      while( arg > 0 ) {
	ucg_com_arduino_port_d_send(data[0], ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_port_d_send(data[0], ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);
	ucg_com_arduino_port_d_send(data[1], ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      while( arg > 0 ) {
	ucg_com_arduino_port_d_send(data[0], ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);
	ucg_com_arduino_port_d_send(data[1], ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);
	ucg_com_arduino_port_d_send(data[2], ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_STR:
      while( arg > 0 ) {
	ucg_com_arduino_port_d_send(*data++, ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      while(arg > 0)
      {
	if ( *data != 0 )
	{
	  if ( *data == 1 )
	  {
	    *ucg->data_port[UCG_PIN_CD] &= ~ucg->data_mask[UCG_PIN_CD];
	  }
	  else
	  {
	    *ucg->data_port[UCG_PIN_CD] |= ucg->data_mask[UCG_PIN_CD];
	  }
	}
	data++;
	ucg_com_arduino_port_d_send(*data, ucg->data_port[UCG_PIN_WR], ~ucg->data_mask[UCG_PIN_WR], ucg->data_mask[UCG_PIN_WR]);	
	data++;
	arg--;
      }
      break;
  }
  return 1;
}

void Ucglib8BitPortD::begin(uint8_t is_transparent)
{ 
  ucg_Init(&ucg, dev_cb, ext_cb, ucg_com_arduino_port_d); 
  ucg_SetFontMode(&ucg, is_transparent);
}

#endif /* __AVR__ */

/*=========================================================================*/

static int16_t ucg_com_arduino_4wire_HW_SPI(ucg_t *ucg, int16_t msg, uint16_t arg, uint8_t *data)
{
  switch(msg)
  {
    case UCG_COM_MSG_POWER_UP:
      /* "data" is a pointer to ucg_com_info_t structure with the following information: */
      /*	((ucg_com_info_t *)data)->serial_clk_speed value in nanoseconds */
      /*	((ucg_com_info_t *)data)->parallel_clk_speed value in nanoseconds */
      
      /* setup pins */
    
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_RST], OUTPUT);
      pinMode(ucg->pin_list[UCG_PIN_CD], OUTPUT);
      
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	pinMode(ucg->pin_list[UCG_PIN_CS], OUTPUT);
      
      /* setup Arduino SPI */
      SPI.begin();
#if defined(__AVR__)
      SPI.setClockDivider( SPI_CLOCK_DIV2 );
#endif
#if defined(__SAM3X8E__)
      SPI.setClockDivider( (((ucg_com_info_t *)data)->serial_clk_speed * 84L + 999)/1000L );
#endif
      SPI.setDataMode(SPI_MODE0);
      SPI.setBitOrder(MSBFIRST);
      break;
    case UCG_COM_MSG_POWER_DOWN:
      SPI.end(); 
      break;
    case UCG_COM_MSG_DELAY:
      delayMicroseconds(arg);
      break;
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      if ( ucg->pin_list[UCG_PIN_RST] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_RST], arg);
      break;
    case UCG_COM_MSG_CHANGE_CS_LINE:
      if ( ucg->pin_list[UCG_PIN_CS] != UCG_PIN_VAL_NONE )
	digitalWrite(ucg->pin_list[UCG_PIN_CS], arg);
      break;
    case UCG_COM_MSG_CHANGE_CD_LINE:
      digitalWrite(ucg->pin_list[UCG_PIN_CD], arg);
      break;
    case UCG_COM_MSG_SEND_BYTE:
      SPI.transfer(arg); 
      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      while( arg > 0 ) {
	SPI.transfer(data[0]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      while( arg > 0 ) {
	SPI.transfer(data[0]);
	SPI.transfer(data[1]);
	arg--;
      }
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      while( arg > 0 ) {
	SPI.transfer(data[0]);
	SPI.transfer(data[1]);
	SPI.transfer(data[2]);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_STR:
      while( arg > 0 ) {
	SPI.transfer(*data++);
	arg--;
      }
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      while(arg > 0)
      {
	if ( *data != 0 )
	{
	  /* set the data line directly, ignore the setting from UCG_CFG_CD */
	  if ( *data == 1 )
	  {
	    digitalWrite(ucg->pin_list[UCG_PIN_CD], 0);
	  }
	  else
	  {
	    digitalWrite(ucg->pin_list[UCG_PIN_CD], 1);
	  }
	}
	data++;
	SPI.transfer(*data);
	data++;
	arg--;
      }
      break;
  }
  return 1;
}

void Ucglib4WireHWSPI::begin(uint8_t is_transparent)
{ 
  ucg_Init(&ucg, dev_cb, ext_cb, ucg_com_arduino_4wire_HW_SPI); 
  ucg_SetFontMode(&ucg, is_transparent);
}


/*=========================================================================*/

void Ucglib::init(void) {
  uint8_t i;
  
  // do a dummy init so that something usefull is part of the ucg structure
  ucg_Init(&ucg, ucg_dev_default_cb, ucg_ext_none, (ucg_com_fnptr)0);

  // reset cursor position
  tx = 0;
  ty = 0;
  tdir = 0;	// default direction for Arduino print() 
  
  for( i = 0; i < UCG_PIN_COUNT; i++ )
    ucg.pin_list[i] = UCG_PIN_VAL_NONE;
  
}

size_t Ucglib::write(uint8_t c) { 
  ucg_int_t delta;
  delta = ucg_DrawGlyph(get_ucg(), get_tx(), get_ty(), get_tdir(), c); 
  switch(get_tdir()) {
    case 0: get_tx() += delta; break;
    case 1: get_ty() += delta; break;
    case 2: get_tx() -= delta; break;
    default: case 3: get_ty() -= delta; break;
  }
  return 1;
}

