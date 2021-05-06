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
char file_read_sensor(const char *filepath, double *line1, double *line2,
                      unsigned char *line3);
char file_read_ac_data(const char *filepath, int *mode, int *power,
                       double *preset, double *measured, int *fan);
#ifdef __cplusplus
}
#endif

bool read_file_to_string(const std::string & p_name, std::string & p_content);

#endif /* W1_H_ */
