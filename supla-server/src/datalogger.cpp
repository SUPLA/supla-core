/*
 ============================================================================
 Name        : datalogger.cpp
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <unistd.h>

#include "datalogger.h"
#include "log.h"
#include "sthread.h"
#include "safearray.h"
#include "user.h"

supla_datalogger::supla_datalogger() {
	this->temperature_tv.tv_sec = 0;
	this->temperature_tv.tv_usec = 0;
	this->db = NULL;
}

void supla_datalogger::log_temperature() {

	supla_user *user;
	int a;
	int n = 0;
	void *tarr = safe_array_init();

	while((user = supla_user::get_user(n)) != NULL) {
		n++;
		user->get_temp_and_humidity(tarr);
	}

	for(a=0;a<safe_array_count(tarr); a++) {

		supla_channel_temphum *sct = (supla_channel_temphum *)safe_array_get(tarr, a);

		if ( sct->isTempAndHumidity() == 1 )
			db->add_temperature_and_humidity(sct->getChannelId(), sct->getTemperature(), sct->getHumidity());
		else
			db->add_temperature(sct->getChannelId(), sct->getTemperature());
	}

	supla_channel_temphum::free(tarr);

}

bool supla_datalogger::dbinit(void) {

	if ( db == NULL ) {
		db = new database();

		if ( !db->connect() ) {
			delete db;
			db = NULL;
		}
	}

	return db != NULL;
}

void supla_datalogger::log(void) {

	gettimeofday(&now, NULL);

	if ( now.tv_sec-temperature_tv.tv_sec >= TEMPLOG_INTERVAL ) {

		temperature_tv = now;

		if ( dbinit() )
			log_temperature();

	}

    if ( db != NULL) {
    	delete db;
    	db = NULL;
    }

}

void datalogger_loop(void *ssd, void *dl_sthread) {

	supla_datalogger *logger = new supla_datalogger();
	database::thread_init();

	while(sthread_isterminated(dl_sthread) == 0) {

		logger->log();
		usleep(1000000);
	}

	delete logger;
	database::thread_end();

}
