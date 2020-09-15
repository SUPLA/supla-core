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

#ifndef WEBHOOK_STATEWEBHOOKCLIENT_H_
#define WEBHOOK_STATEWEBHOOKCLIENT_H_

#include "json/cJSON.h"
#include "webhook/statewebhookcredentials.h"
#include "webhook/webhookbasicclient.h"

class supla_state_webhook_client : public supla_webhook_basic_client {
 private:
  supla_state_webhook_credentials *getStateWebhookCredentials(void);
  void refreshToken(void);
  bool postRequest(const char *data, int *httpResultCode);
  bool postRequest(const char *data);
  bool sendReport(cJSON *json);
  cJSON *getHeader(const char *function, int channelId);
  bool sendReportWithBool(const char *function, int channelId, const char *name,
                          bool _true, bool connected);
  bool sendReportWithNumber(const char *function, int channelId,
                            const char *name, double number, bool connected);
  bool sendOnReport(const char *function, int channelId, bool on,
                    bool connected);
  bool sendHiReport(const char *function, int channelId, bool hi,
                    bool connected);
  bool sendValueReport(const char *function, int channelId, double number,
                       bool connected);
  bool sendTemperatureAndHumidityReport(const char *function, int channelId,
                                        double *temperature, double *humidity,
                                        bool connected);

 public:
  supla_state_webhook_client(supla_webhook_basic_credentials *credentials);
  bool sendLightSwitchReport(int channelId, bool on, bool connected);
  bool sendPowerSwitchReport(int channelId, bool on, bool connected);

  bool sendTemperatureReport(int channelId, double temperature, bool connected);
  bool sendHumidityReport(int channelId, double humidity, bool connected);
  bool sendTemperatureAndHumidityReport(int channelId, double temperature,
                                        double humidity, bool connected);
  bool sendGatewayOpeningSensorReport(int channelId, bool hi, bool connected);
  bool sendGateOpeningSensorReport(int channelId, bool hi, bool connected);
  bool sendGarageDoorOpeningSensorReport(int channelId, bool hi,
                                         bool connected);
  bool sendNoLiquidSensorReport(int channelId, bool hi, bool connected);
  bool sendDoorOpeningSensorReport(int channelId, bool hi, bool connected);
  bool sendRollerShutterOpeningSensorReport(int channelId, bool hi,
                                            bool connected);
  bool sendWindowOpeningSensorReport(int channelId, bool hi, bool connected);
  bool sendMailSensorReport(int channelId, bool hi, bool connected);
  bool sendRollerShutterReport(int channelId, char shut, bool connected);
  bool sendWindSensorReport(int channelId, double value, bool connected);
};

#endif /* WEBHOOK_STATEWEBHOOKCLIENT_H_ */
