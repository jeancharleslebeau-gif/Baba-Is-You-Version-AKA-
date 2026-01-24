/*
This file is part of the Gamebuino-AKA library,
Copyright (c) Gamebuino 2026

This is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License (LGPL) for more details.

You should have received a copy of the GNU Lesser General Public
License (LGPL) along with the library.
If not, see <http://www.gnu.org/licenses/>.

Authors:
 - Jean-Marie Papillon
*/
#include "gb_core.h"
#include "freertos/FreeRTOS.h"
#include "gb_ll_lcd.h"
#include "gb_ll_i2c.h"
#include "gb_ll_expander.h"
#include "gb_ll_i2c.h"
#include "gb_ll_audio.h"
#include "gb_ll_adc.h"
#include "gb_ll_sdcard.h"
#include "gb_ll_system.h"


gb_core::gb_core() {

}

gb_core::~gb_core() {

}


    //! delay in milliseconds
void gb_core::delay_ms(uint32_t u32_ms)
{
    gb_delay_ms(u32_ms);
}
    //! return date in milliseconds
uint32_t gb_core::get_millis()
{
    return gb_get_millis();
}

    //! return date in microseconds
int64_t gb_core::get_micros()
{
    return gb_get_micros();
}



void gb_core::init()
{
    gb_ll_system_init();
    gb_ll_adc_init();
    gb_ll_i2c_init();
    gb_ll_expander_init();
    gb_ll_sd_init();
    gb_ll_lcd_init();
    gb_ll_audio_init();
}


void gb_buttons::update()
{
    u16_buttons_last = u16_buttons;
    u16_buttons = gb_ll_expander_read() & EXPANDER_KEY;
    if (u16_buttons & EXPANDER_KEY_RUN)
        gb_ll_expander_power_off();
}

uint16_t gb_buttons::state()
{
    return u16_buttons;
}

uint16_t gb_buttons::pressed()
{
    return u16_buttons & (~u16_buttons_last);
}

bool gb_buttons::pressed( gb_key key )
{
    return (pressed()&key)?true:false;
}

uint16_t gb_buttons::released()
{
    return (~u16_buttons) & u16_buttons_last;
}

bool gb_buttons::released( gb_key key )
{
    return (released()&key)?true:false;
}


int16_t gb_joystick::get_x()
{
    return i16_joy_x;
}

int16_t gb_joystick::get_y()
{
    return i16_joy_y;
}

void gb_joystick::update()
{
    i16_joy_x = 2000*(gb_ll_adc_read_joyx()-JOYX_MID)/JOYX_MAX;
    i16_joy_y = 2000*(gb_ll_adc_read_joyy()-JOYX_MID)/JOYX_MAX;;
}


void gb_core::pool()
{
    buttons.update();
    joystick.update();
}




size_t gb_core::free_psram()
{
//    printf("PSRAM Total %u\n", heap_caps_get_total_size(MALLOC_CAP_SPIRAM) );
//    printf("PSRAM Free  %u\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) );
//    printf("PSRAM max bloc %u\n", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM) );
    return heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
}

size_t gb_core::free_sram()
{
//    printf("SRAM Total %u\n", heap_caps_get_total_size(MALLOC_CAP_8BIT) );
//    printf("SRAM Free  %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT) );
//    printf("SRAM max bloc %u\n", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) );    
    return heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
}

            //! power down 
void gb_core::power_down()
{
    gb_ll_expander_power_off();
}

gb_core g_core;
