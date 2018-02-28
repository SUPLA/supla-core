/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef CHANNEL_IO_H_
#define CHANNEL_IO_H_

#include "proto.h"

typedef void (*_func_channelio_valuechanged)(
    unsigned char number, char value[SUPLA_CHANNELVALUE_SIZE], void *user_data);

#ifdef __cplusplus
extern "C" {
#endif

char channelio_init(void);
void channelio_free(void);
void channelio_channel_init(void);
int channelio_channel_count(void);
void channelio_set_type(unsigned char number, int type);
int channelio_get_type(unsigned char number);
void channelio_set_gpio1(unsigned char number, unsigned char gpio1);
void channelio_set_gpio2(unsigned char number, unsigned char gpio2);
void channelio_set_bistable_flag(unsigned char number, unsigned char bistable);
void channelio_set_w1(unsigned char number, const char *w1);
void channelio_set_mcp23008_driver(unsigned char number, int driver);
void channelio_set_mcp23008_addr(unsigned char number, unsigned char addr);
void channelio_set_mcp23008_reset(unsigned char number, unsigned char reset);
void channelio_set_mcp23008_gpio_dir(unsigned char number, unsigned char value);
void channelio_set_mcp23008_gpio_val(unsigned char number, unsigned char value);
void channelio_set_mcp23008_gpio_port(unsigned char number, unsigned char port);
char channelio_get_value(unsigned char number,
                         char value[SUPLA_CHANNELVALUE_SIZE]);
char channelio_get_hi_value(unsigned char number, char *hi);
char channelio_set_hi_value(unsigned char number, char hi,
                            unsigned int time_ms);
void channelio_channels_to_srd(unsigned char *channel_count,
                               TDS_SuplaDeviceChannel_B *channels);

// TMP TEST
void tmp_channelio_raise_valuechanged(unsigned char number);

void channelio_setcalback_on_channel_value_changed(
    _func_channelio_valuechanged on_valuechanged, void *user_data);

#ifdef __SINGLE_THREAD
void channelio_iterate(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* CHANNEL_IO_H_ */
