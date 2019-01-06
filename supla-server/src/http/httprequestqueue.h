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

#ifndef HTTP_HTTPREQUESTQUEUE_H_
#define HTTP_HTTPREQUESTQUEUE_H_

#include "commontypes.h"
#include "eh.h"
#include "string.h"

class supla_http_request;
class supla_user;

typedef struct {
  supla_user *user;
  void *arr_queue;
} _heq_user_space_t;

class supla_http_request_queue {
 private:
  static supla_http_request_queue *instance;
  TEventHandler *main_eh;
  void *arr_user_space;
  void *arr_thread;
  int thread_count_limit;
  int last_user_offset;

  void terminateAllThreads(void);
  void runThread(supla_http_request *request);
  _heq_user_space_t *getUserSpace(supla_user *user);
  void addRequest(_heq_user_space_t *user_space, supla_http_request *request);
  supla_http_request *queuePop(void);
  int getNextTimeOfDelayedExecution(int time);
  int queueSize(void);
  int threadCount(void);
  int threadCountLimit(void);
  void createByChannelEventSourceType(supla_user *user, int deviceId,
                                      int channelId, event_type eventType,
                                      event_source_type eventSourceType,
                                      const char correlationToken[],
                                      const char googleRequestId[]);

 public:
  static void init();
  static void queueFree();
  static supla_http_request_queue *getInstance(void);
  supla_http_request_queue();
  virtual ~supla_http_request_queue();

  void raiseEvent(void);
  void iterate(void *q_sthread);
  void addRequest(supla_http_request *request);
  void onChannelValueChangeEvent(supla_user *user, int deviceId, int channelId,
                                 event_source_type eventSourceType,
                                 const char correlationToken[] = NULL,
                                 const char googleRequestId[] = NULL);

  void onDeviceAddedEvent(supla_user *user, int deviceId,
                          event_source_type eventSourceType,
                          const char correlationToken[] = NULL,
                          const char googleRequestId[] = NULL);

  void onDeviceDeletedEvent(supla_user *user, int deviceId,
                            event_source_type eventSourceType,
                            const char correlationToken[] = NULL,
                            const char googleRequestId[] = NULL);
};

void http_request_queue_loop(void *ssd, void *q_sthread);

#endif /* HTTP_HTTPREQUESTQUEUE_H_ */
