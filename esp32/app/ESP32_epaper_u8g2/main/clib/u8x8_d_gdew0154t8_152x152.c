/** @file  u8x8_d_gdew0154t8_152x152.c

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  e-paper driver
  Copyright (c) 2017 pcbreflux. All Rights Reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
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
  
 * @brief  GDEW0154T8: 152x152x1 u8g2 driver
  
  Notes:
    - Introduced a refresh display message, which copies RAM to display
    - Charge pump and clock are only enabled for the transfer RAM to display
    - U8x8 will not really work because of the two buffers in the GDEW0154T8, however U8g2 should be ok.

*/


#include <stdio.h>
#include "u8x8.h"

/**@brief gdew0154t8 output buffer in display order. */
static uint8_t u8x8_d_gdew0154t8_152x152_outbuf[2888];

/**@brief gdew0154t8 init Sequence . */
static const uint8_t u8x8_d_gdew0154t8_152x152_init_seq[] = {
    
	U8X8_START_TRANSFER(),            /** enable chip, delay is part of the transfer start */
	U8X8_CAAA(0x06,0x17,0x17,0x17),   /**  Boost setup */
	U8X8_C(0x04),                     /**  Power on */
	U8X8_DLY(250),		              /** Instead of wait for busy line, but we rely a delay! */
	U8X8_DLY(250),
	U8X8_DLY(250),
	U8X8_DLY(250),
	U8X8_CAA(0x00,0x1F,0x0B),		   /** Panel setting */
	U8X8_CAAA(0x61,0x98,0x00,0x98),    /** Resolution setting 152x152 */
	U8X8_CA(0x50,0x97),                /** VCOM AND DATA */
	U8X8_END_TRANSFER(),               /** disable chip */
	U8X8_END()             			   /** end of sequence */
};

/**@brief gdew0154t8 send data to Display Sequence . */
static const uint8_t u8x8_d_gdew0154t8_to_display_seq[] = {
	U8X8_START_TRANSFER(),             /** enable chip, delay is part of the transfer start */
	U8X8_C(0x12),                      /** Data Refresh Command */
	U8X8_DLY(250),
	U8X8_DLY(250),
	U8X8_END_TRANSFER(),               /** disable chip */
	U8X8_END()             			   /** end of sequence */
};

/**@brief gdew0154t8 wake up Sequence . */
static const uint8_t u8x8_d_gdew0154t8_152x152_powersave0_seq[] = {
	U8X8_START_TRANSFER(),             /** enable chip, delay is part of the transfer start */
	U8X8_END_TRANSFER(),               /** disable chip */
	U8X8_END()             			   /** end of sequence */
};

/**@brief gdew0154t8 power down Sequence . */
static const uint8_t u8x8_d_gdew0154t8_152x152_powersave1_seq[] = {
	U8X8_START_TRANSFER(),             	/** enable chip, delay is part of the transfer start */
	U8X8_CA(0x50,0x17),                 /** Vcom and data interval setting, BD floating */
	U8X8_C(0x82),                       /** VCM_DC setting 0V */
	U8X8_C(0x00),                       /** VCM_DC setting 0V */
	U8X8_DLY(100),
	U8X8_CAAAA(0x01,0x00,0x00,0x00,0x00), /** Power Setting gate switch to external */
	U8X8_DLY(100),
	U8X8_C(0x02),                        /** power off */
	U8X8_DLY(150),
	U8X8_DLY(150),
	U8X8_CA(0x07,0xA5),                  /** deep sleep */
	U8X8_END_TRANSFER(),             	 /** disable chip */
	U8X8_END()             			     /** end of sequence */
};

/**@brief gdew0154t8 flip Sequence . */
static const uint8_t u8x8_d_gdew0154t8_152x152_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/** enable chip, delay is part of the transfer start */
  U8X8_END_TRANSFER(),             	    /** disable chip */
  U8X8_END()             			    /** end of sequence */
};

/**@brief gdew0154t8 flip Sequence . */
static const uint8_t u8x8_d_gdew0154t8_152x152_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/** enable chip, delay is part of the transfer start */
  U8X8_END_TRANSFER(),             	    /** disable chip */
  U8X8_END()             			    /** end of sequence */
};

/**@brief Function for counter clock wise rotate 8x8 bit.
 *
 * @param[in] t pointer to 8x8 bit.
 *
 * @retval pointer to 8x8 rotated bit.
 */
static uint8_t *u8x8_ccw_rotate_for_gdew0154t8(uint8_t *t) {
  uint8_t bytepos,bitpos;
  static uint8_t buf[8];

  for( bytepos = 0; bytepos < 8; bytepos++ ) {
	  for( bitpos = 0; bitpos < 8; bitpos++ ) {
//		  buf[bytepos] = (buf[bytepos] << 1) | ((t[bitpos] >> (7 - bytepos)) & 0x01);
		  buf[bytepos] = (buf[bytepos] << 1) | ((t[bitpos] >> bytepos) & 0x01);
	  }
  }
  return buf;
}

/**@brief Function to copy many 8x8 tiles to outputbuffer.
 *
 * @param[in] u8x8 pointer to 8x8 structure.
 * @param[in] arg_int number of arguments.
 * @param[in] arg_ptr arguments.
 */
static void u8x8_d_gdew0154t8_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr) U8X8_NOINLINE;
static void u8x8_d_gdew0154t8_draw_tile(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr) {
  uint16_t x,y;
  uint8_t c;
  uint8_t *ptr,*ptrconv;

  u8x8_cad_StartTransfer(u8x8);

  uint8_t hight=u8x8->display_info->pixel_height;
  uint8_t tile_width=u8x8->display_info->tile_width;

  c = ((u8x8_tile_t *)arg_ptr)->cnt;
  y = ((u8x8_tile_t *)arg_ptr)->y_pos;
  ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;

  for (x=0;x<c;x++) {
	  ptrconv=u8x8_ccw_rotate_for_gdew0154t8(ptr);
	  for (uint8_t block=0;block<8;block++) {
		  u8x8_d_gdew0154t8_152x152_outbuf[x+y*hight+block*tile_width]=ptrconv[block];
	  }
      ptr +=8;
  }

  u8x8_cad_EndTransfer(u8x8);
}

/**@brief Function to send outputbuffer to display.
 *
 * @param[in] u8x8 pointer to 8x8 structure.
 * @param[in] arg_int number of arguments.
 * @param[in] arg_ptr arguments.
 */
static void u8x8_d_gdew0154t8_display_refresh(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr) U8X8_NOINLINE;
static void u8x8_d_gdew0154t8_display_refresh(u8x8_t *u8x8, uint8_t arg_int, void *arg_ptr) {
	  uint8_t hight=u8x8->display_info->pixel_height;
	  uint8_t tile_width=u8x8->display_info->tile_width;
	  uint8_t buf;

	  u8x8_cad_SendCmd(u8x8,0x13);  // just sending red (!?!)
	  for (uint8_t y=0;y<hight;y++) {
		  for (uint8_t x=0;x<tile_width;x++) {
			  buf=u8x8_d_gdew0154t8_152x152_outbuf[x+y*tile_width]^0xFF; // inverse with XOR
		      u8x8_cad_SendData(u8x8, 1, &buf);  // just sending one byte
		  }
	  }
}

/**@brief Function for message callback handling.
 *
 * @param[in] u8x8 pointer to 8x8 structure.
 * @param[in] msg message.
 * @param[in] arg_int number of arguments.
 * @param[in] arg_ptr arguments.
 */
static uint8_t u8x8_d_gdew0154t8_152x152_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  switch(msg) {
    case U8X8_MSG_DISPLAY_INIT:
      /* single shot mode */
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
        /* single shot mode */
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
    	/* nope */
      break;
#ifdef U8X8_WITH_SET_CONTRAST
  	/* nope */
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_d_gdew0154t8_draw_tile(u8x8, arg_int, arg_ptr);
      break;
    case U8X8_MSG_DISPLAY_REFRESH:
    {
        /* single shot mode */
        u8x8_d_helper_display_init(u8x8);
        u8x8_cad_SendSequence(u8x8, u8x8_d_gdew0154t8_152x152_init_seq);
    	u8x8_d_gdew0154t8_display_refresh(u8x8, arg_int, arg_ptr);
		u8x8_cad_SendSequence(u8x8, u8x8_d_gdew0154t8_to_display_seq);
		u8x8_cad_SendSequence(u8x8, u8x8_d_gdew0154t8_152x152_powersave1_seq);
      break;
    }
    default:
      return 0;
  }
  return 1;
}

/**@brief gdew0154t8 display parameter. */
static const u8x8_display_info_t u8x8_gdew0154t8_152x152_display_info = {
  0,         /** chip_enable_level */
  1,         /** chip_disable_level */
  120,       /** post_chip_enable_wait_ns */
  60,        /** pre_chip_disable_wait_ns */
  255,       /** reset_pulse_width_ms */
  255,       /** post_reset_wait_ms */
  50,        /** sda_setup_time_ns */
  100,       /** sck_pulse_width_ns = */
  4000000UL, /** sck_clock_hz since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  0,		 /** spi_mode, active high, rising edge */
  4,         /** i2c_bus_clock_100kHz */
  40,        /** data_setup_time_ns = */
  150,       /** write_pulse_width_ns */
  19,        /** tile_width 19*8 = 152 */
  19,        /** tile_hight 19*8 = 152 */
  0,         /** default_x_offset */
  0,         /** flipmode_x_offset */
  152,       /** pixel_width */
  152        /** pixel_height */
};

/**@brief Function for display setup.
 *
 * @param[in] u8x8 pointer to 8x8 structure.
 * @param[in] msg message.
 * @param[in] arg_int number of arguments.
 * @param[in] arg_ptr arguments.
 */
uint8_t u8x8_d_gdew0154t8_152x152(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY ) {
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_gdew0154t8_152x152_display_info);
      return 1;
    }
    return u8x8_d_gdew0154t8_152x152_generic(u8x8, msg, arg_int, arg_ptr);
}


