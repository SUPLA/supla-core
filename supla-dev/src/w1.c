/*
 ============================================================================
 Name        : w1.c [1-WIRE]
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "w1.h"
#include "tools.h"
#include "log.h"
#include "pi_2_dht_read.h"

#define W1_DEFAULT_PIN 4

char w1_ds18b20_get_temp(char *device, double *temp) {

   int fd;
   int count;
   int val;
   char buffer[200];
   char result = 0;

   if ( device == 0
        || strlen(device) == 0
        || strlen(device) > 100 )
     return 0;

   memset(buffer, 0, 200);

#ifdef __W1_SIMULATE
   snprintf(buffer, 200, "/supla_virtual_dev/w1/devices/%s/w1_slave", device);
#else
   snprintf(buffer, 200, "/sys/bus/w1/devices/%s/w1_slave", device);
#endif

   if ( st_file_exists(buffer) == 0 )
     return 0;

   fd = open(buffer, O_RDONLY);
   if ( fd != -1 ) {
        if ( 70 <= ( count = read(fd, buffer, 200) )
             && 0 == memcmp(&buffer[29], "crc", 3)
             && 0 == memcmp(&buffer[36], "YES", 3)
             && 0 == memcmp(&buffer[67], "t=", 2)  ) {

              sscanf(&buffer[69], "%i", &val);
              *temp = val/1000.00;

              result = 1;
        };
        close(fd);
   };


   return result;
}


//https://github.com/adafruit/Adafruit-Raspberry-Pi-Python-Code/blob/master/Adafruit_DHT_Driver/Adafruit_DHT.c

char w1_dht_read(const char *w1, double *temp, double *humidity, char chip_type) {

	int pin = W1_DEFAULT_PIN;

	if ( w1 != NULL ) {
		pin = atoi(w1);

		if ( pin == 0 )
			pin = W1_DEFAULT_PIN;
	}



#ifdef __W1_SIMULATE
	*temp = 22.543;
	*humidity =  40.321;
	return 1;
#else

	{
		float _humidity = 0, _temperature = 0;
		int result = pi_2_dht_read(chip_type, pin, &_humidity, &_temperature);

		if ( result == 0 ) {

			*temp = _temperature;
			*humidity = _humidity;

			return 1;
		}
	}

	*temp = -275;
	*humidity = -1;

	return 0;
#endif
}

