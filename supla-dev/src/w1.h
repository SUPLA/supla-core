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

#ifndef W1_H_
#define W1_H_

#ifdef __cplusplus
extern "C" {
#endif

char w1_ds18b20_get_temp(char *device, double *temp);

// DHT11, DHT22, AM2302
char w1_dht_read(const char *w1, double *temp, double *humidity,
                 char chip_type);

#ifdef __cplusplus
}
#endif

#endif /* W1_H_ */
