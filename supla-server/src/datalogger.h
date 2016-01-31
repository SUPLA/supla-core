/*
 ============================================================================
 Name        : datalogger.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef DATALOGGER_H_
#define DATALOGGER_H_

#include "db.h"

#define TEMPLOG_INTERVAL 600

class supla_datalogger {
private:

	database *db;
	struct timeval now;
	struct timeval temperature_tv;

	void log_temperature();
	bool dbinit(void);

public:
	supla_datalogger();
	void log(void);
};

void datalogger_loop(void *ssd, void *dl_sthread);


#endif /* DATALOGGER_H_ */
