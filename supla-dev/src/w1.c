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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "w1.h"
#include "tools.h"

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
