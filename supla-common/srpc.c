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

#include "srpc.h"

#include <stdlib.h>
#include <string.h>

#include "lck.h"
#include "log.h"
#include "proto.h"

// ESP8266 and ESP32
#if defined(ESP8266) || defined(ESP32)
#ifndef __EH_DISABLED
#define __EH_DISABLED
#endif /*__EH_DISABLED*/
#define SRPC_BUFFER_SIZE 256
#define SRPC_QUEUE_SIZE 2
#define SRPC_QUEUE_MIN_ALLOC_COUNT 2

#if !defined(ESP32)
#include <mem.h>
#include <osapi.h>
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include <ets_sys.h>
#elif defined(ARDUINO_ARCH_ESP32)
#else
// ESP8266 nonos SDK target
#include <user_interface.h>

#include "espmissingincludes.h"
#endif

// ARDUINO MEGA
#elif defined(__AVR__)
#define SRPC_BUFFER_SIZE 32
#define SRPC_QUEUE_SIZE 1
#define SRPC_QUEUE_MIN_ALLOC_COUNT 1
#define __EH_DISABLED

// OTHER SUPLA_DEVICE
// Linux target, other?
#elif defined(SUPLA_DEVICE)
#define SRPC_BUFFER_SIZE 1024
#define __EH_DISABLED

// other not releated to supla-device
#else
#include <assert.h>
#endif

// ALL targets
#ifndef SRPC_BUFFER_SIZE
#define SRPC_BUFFER_SIZE 32768
#endif /*SRPC_BUFFER_SIZE*/

#ifndef SRPC_QUEUE_SIZE
#define SRPC_QUEUE_SIZE 10
#endif /*SRPC_QUEUE_SIZE*/

#ifndef SRPC_QUEUE_MIN_ALLOC_COUNT
#define SRPC_QUEUE_MIN_ALLOC_COUNT 0
#endif /*SRPC_QUEUE_MIN_ALLOC_COUNT*/

typedef struct {
  unsigned char item_count;
  unsigned char alloc_count;

  TSuplaDataPacket *item[SRPC_QUEUE_SIZE];
} Tsrpc_Queue;

typedef struct {
  void *proto;
  TsrpcParams params;

  TSuplaDataPacket sdp;

#ifndef SRPC_WITHOUT_IN_QUEUE
  Tsrpc_Queue in_queue;
#endif /*SRPC_WITHOUT_IN_QUEUE*/

#ifndef SRPC_WITHOUT_OUT_QUEUE
  Tsrpc_Queue out_queue;
#endif /*SRPC_WITHOUT_OUT_QUEUE*/

  void *lck;
} Tsrpc;

void SRPC_ICACHE_FLASH srpc_get_scene_pack(Tsrpc *srpc, TsrpcReceivedData *rd);
void SRPC_ICACHE_FLASH srpc_get_scene_state_pack(Tsrpc *srpc,
                                                 TsrpcReceivedData *rd);

void SRPC_ICACHE_FLASH srpc_params_init(TsrpcParams *params) {
  memset(params, 0, sizeof(TsrpcParams));
}

void *SRPC_ICACHE_FLASH srpc_init(TsrpcParams *params) {
  Tsrpc *srpc = (Tsrpc *)malloc(sizeof(Tsrpc));

  if (srpc == NULL) return NULL;

  memset(srpc, 0, sizeof(Tsrpc));
  srpc->proto = sproto_init();

#ifndef ESP8266
#ifndef ESP32
#ifndef __AVR__
#ifndef SUPLA_DEVICE
  assert(params != 0);
  assert(params->data_read != 0);
  assert(params->data_write != 0);
#endif
#endif
#endif
#endif

  memcpy(&srpc->params, params, sizeof(TsrpcParams));

  srpc->lck = lck_init();

  return srpc;
}

void SRPC_ICACHE_FLASH srpc_lock(void *_srpc) {
  lck_lock(((Tsrpc *)_srpc)->lck);
}

void SRPC_ICACHE_FLASH srpc_unlock(void *_srpc) {
  lck_unlock(((Tsrpc *)_srpc)->lck);
}

void SRPC_ICACHE_FLASH srpc_queue_free(Tsrpc_Queue *queue) {
  _supla_int_t a;
  for (a = 0; a < SRPC_QUEUE_SIZE; a++) {
    if (queue->item[a] != NULL) {
      free(queue->item[a]);
    }
  }

  queue->item_count = 0;
  queue->alloc_count = 0;
}

void SRPC_ICACHE_FLASH srpc_free(void *_srpc) {
  if (_srpc) {
    Tsrpc *srpc = (Tsrpc *)_srpc;

    sproto_free(srpc->proto);

#ifndef SRPC_WITHOUT_IN_QUEUE
    srpc_queue_free(&srpc->in_queue);
#endif /*SRPC_WITHOUT_IN_QUEUE*/

#ifndef SRPC_WITHOUT_OUT_QUEUE
    srpc_queue_free(&srpc->out_queue);
#endif /*SRPC_WITHOUT_OUT_QUEUE*/
    lck_free(srpc->lck);

    free(srpc);
  }
}

char SRPC_ICACHE_FLASH srpc_queue_push(Tsrpc_Queue *queue,
                                       TSuplaDataPacket *sdp) {
  if (queue->item_count >= SRPC_QUEUE_SIZE) {
    return SUPLA_RESULT_FALSE;
  }

  if (queue->item[queue->item_count] == NULL) {
    queue->item[queue->item_count] =
        (TSuplaDataPacket *)malloc(sizeof(TSuplaDataPacket));
  }

  if (queue->item[queue->item_count] == NULL) {
    return SUPLA_RESULT_FALSE;
  } else {
    queue->alloc_count++;
  }

  memcpy(queue->item[queue->item_count], sdp, sizeof(TSuplaDataPacket));
  queue->item_count++;

  return SUPLA_RESULT_TRUE;
}

char SRPC_ICACHE_FLASH srpc_queue_pop(Tsrpc_Queue *queue, TSuplaDataPacket *sdp,
                                      unsigned _supla_int_t rr_id) {
  _supla_int_t a, b;

  for (a = 0; a < queue->item_count; a++)
    if (rr_id == 0 || queue->item[a]->rr_id == rr_id) {
      memcpy(sdp, queue->item[a], sizeof(TSuplaDataPacket));

      if (queue->alloc_count > SRPC_QUEUE_MIN_ALLOC_COUNT) {
        queue->alloc_count--;
        free(queue->item[a]);
        queue->item[a] = NULL;
      }

      TSuplaDataPacket *item = queue->item[a];

      for (b = a; b < queue->item_count - 1; b++) {
        queue->item[b] = queue->item[b + 1];
      }

      queue->item_count--;
      queue->item[queue->item_count] = item;

      return SUPLA_RESULT_TRUE;
    }

  return SUPLA_RESULT_FALSE;
}

char SRPC_ICACHE_FLASH srpc_in_queue_pop(Tsrpc *srpc, TSuplaDataPacket *sdp,
                                         unsigned _supla_int_t rr_id) {
  (void)(srpc);
  (void)(sdp);
  (void)(rr_id);
#ifdef SRPC_WITHOUT_IN_QUEUE
  return 1;
#else
  return srpc_queue_pop(&srpc->in_queue, sdp, rr_id);
#endif /*SRPC_WITHOUT_IN_QUEUE*/
}

#ifndef SRPC_WITHOUT_IN_QUEUE
char SRPC_ICACHE_FLASH srpc_in_queue_push(Tsrpc *srpc, TSuplaDataPacket *sdp) {
  return srpc_queue_push(&srpc->in_queue, sdp);
}
#endif /*SRPC_WITHOUT_IN_QUEUE*/

char SRPC_ICACHE_FLASH srpc_out_queue_push(Tsrpc *srpc, TSuplaDataPacket *sdp) {
#ifdef SRPC_WITHOUT_OUT_QUEUE
  unsigned _supla_int_t data_size = sizeof(TSuplaDataPacket);
  if (sdp->data_size < SUPLA_MAX_DATA_SIZE) {
    data_size -= SUPLA_MAX_DATA_SIZE - sdp->data_size;
  }
#ifndef PACKET_INTEGRITY_BUFFER_DISABLED
  char *buff = malloc(data_size + SUPLA_TAG_SIZE);
  if (buff) {
    memcpy(buff, sdp, data_size);
    memcpy(&buff[data_size], sproto_tag, SUPLA_TAG_SIZE);

    srpc->params.data_write(buff, data_size + SUPLA_TAG_SIZE,
                            srpc->params.user_params);
    free(buff);
  }
#else
  srpc->params.data_write((char *)sdp, data_size, srpc->params.user_params);
  srpc->params.data_write(sproto_tag, SUPLA_TAG_SIZE, srpc->params.user_params);
#endif /*PACKET_INTEGRITY_BUFFER_DISABLED*/
  return 1;
#else
  return srpc_queue_push(&srpc->out_queue, sdp);
#endif /*SRPC_WITHOUT_OUT_QUEUE*/
}

#ifndef SRPC_WITHOUT_OUT_QUEUE
char SRPC_ICACHE_FLASH srpc_out_queue_pop(Tsrpc *srpc, TSuplaDataPacket *sdp,
                                          unsigned _supla_int_t rr_id) {
  return srpc_queue_pop(&srpc->out_queue, sdp, rr_id);
}
#endif /*SRPC_WITHOUT_OUT_QUEUE*/

unsigned char SRPC_ICACHE_FLASH srpc_out_queue_item_count(void *srpc) {
  (void)(srpc);
#ifdef SRPC_WITHOUT_OUT_QUEUE
  return 0;
#else
  return ((Tsrpc *)srpc)->out_queue.item_count;
#endif /*SRPC_WITHOUT_OUT_QUEUE*/
}

char SRPC_ICACHE_FLASH srpc_input_dataexists(void *_srpc) {
  int result = SUPLA_RESULT_FALSE;
  Tsrpc *srpc = (Tsrpc *)_srpc;
  lck_lock(srpc->lck);
  result = sproto_in_dataexists(srpc->proto);
  return lck_unlock_r(srpc->lck, result);
}

char SRPC_ICACHE_FLASH srpc_output_dataexists(void *_srpc) {
  int result = SUPLA_RESULT_FALSE;
  Tsrpc *srpc = (Tsrpc *)_srpc;
  lck_lock(srpc->lck);
  result = sproto_out_dataexists(srpc->proto);
  return lck_unlock_r(srpc->lck, result);
}

char SRPC_ICACHE_FLASH srpc_iterate(void *_srpc) {
  Tsrpc *srpc = (Tsrpc *)_srpc;
  char data_buffer[SRPC_BUFFER_SIZE];
  char result;
  unsigned char version;
#ifndef __EH_DISABLED
  unsigned char raise_event = 0;
#endif /*__EH_DISABLED*/

  // --------- IN ---------------
  lck_lock(srpc->lck);
  _supla_int_t data_size = srpc->params.data_read(data_buffer, SRPC_BUFFER_SIZE,
                                                  srpc->params.user_params);

  if (data_size == 0) {
    return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
  }

  if (data_size > 0 &&
      SUPLA_RESULT_TRUE != (result = sproto_in_buffer_append(
                                srpc->proto, data_buffer, data_size))) {
    supla_log(LOG_DEBUG, "sproto_in_buffer_append: %i, datasize: %i", result,
              data_size);
    return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
  }

  if (SUPLA_RESULT_TRUE ==
      (result = sproto_pop_in_sdp(srpc->proto, &srpc->sdp))) {
#ifndef __EH_DISABLED
    raise_event = sproto_in_dataexists(srpc->proto) == 1 ? 1 : 0;
#endif /*__EH_DISABLED*/

#ifdef SRPC_WITHOUT_IN_QUEUE
    if (srpc->params.on_remote_call_received) {
      lck_unlock(srpc->lck);
      srpc->params.on_remote_call_received(
          srpc, srpc->sdp.rr_id, srpc->sdp.call_id, srpc->params.user_params,
          srpc->sdp.version);
      lck_lock(srpc->lck);
    }
#else
    if (SUPLA_RESULT_TRUE == srpc_in_queue_push(srpc, &srpc->sdp)) {
      if (srpc->params.on_remote_call_received) {
        lck_unlock(srpc->lck);
        srpc->params.on_remote_call_received(
            srpc, srpc->sdp.rr_id, srpc->sdp.call_id, srpc->params.user_params,
            srpc->sdp.version);
        lck_lock(srpc->lck);
      }

    } else {
      supla_log(LOG_DEBUG, "ssrpc_in_queue_push error");
      return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
    }
#endif /*SRPC_WITHOUT_IN_QUEUE*/

  } else if (result != SUPLA_RESULT_FALSE) {
    if (result == (char)SUPLA_RESULT_VERSION_ERROR) {
      if (srpc->params.on_version_error) {
        version = srpc->sdp.version;
        lck_unlock(srpc->lck);

        srpc->params.on_version_error(srpc, version, srpc->params.user_params);
        return SUPLA_RESULT_FALSE;
      }
    } else {
      supla_log(LOG_DEBUG, "sproto_pop_in_sdp error: %i", result);
    }

    return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
  }

  // --------- OUT ---------------
#ifndef SRPC_WITHOUT_OUT_QUEUE
  if (srpc_out_queue_pop(srpc, &srpc->sdp, 0) == SUPLA_RESULT_TRUE &&
      SUPLA_RESULT_TRUE !=
          (result = sproto_out_buffer_append(srpc->proto, &srpc->sdp)) &&
      result != SUPLA_RESULT_FALSE) {
    supla_log(LOG_DEBUG, "sproto_out_buffer_append error: %i", result);
    return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
  }

  data_size = sproto_pop_out_data(srpc->proto, data_buffer, SRPC_BUFFER_SIZE);

  if (data_size != 0) {
    srpc->params.data_write(data_buffer, data_size, srpc->params.user_params);
  }

#ifndef __EH_DISABLED
  if (srpc->params.eh != 0 &&
      (sproto_out_dataexists(srpc->proto) == 1 ||
       srpc_out_queue_item_count(srpc) || raise_event)) {
    eh_raise_event(srpc->params.eh);
  }
#endif /*__EH_DISABLED*/

#else /*SRPC_WITHOUT_OUT_QUEUE*/
#ifndef __EH_DISABLED
  if (srpc->params.eh != 0 && raise_event) {
    eh_raise_event(srpc->params.eh);
  }
#endif /*__EH_DISABLED*/
#endif /*SRPC_WITHOUT_OUT_QUEUE*/
  return lck_unlock_r(srpc->lck, SUPLA_RESULT_TRUE);
}

typedef unsigned _supla_int_t (*_func_srpc_pack_get_caption_size)(
    void *pack, _supla_int_t idx);
typedef void *(*_func_srpc_pack_get_item_ptr)(void *pack, _supla_int_t idx);
typedef _supla_int_t (*_func_srpc_pack_get_pack_count)(void *pack);
typedef void (*_func_srpc_pack_set_pack_count)(void *pack, _supla_int_t count,
                                               unsigned char increment);
typedef unsigned _supla_int_t (*_func_srpc_pack_get_item_caption_size)(
    void *item);
typedef unsigned _supla_int_t (*_func_srpc_pack_get_caption_size)(
    void *pack, _supla_int_t idx);

void SRPC_ICACHE_FLASH srpc_getpack(
    Tsrpc *srpc, TsrpcReceivedData *rd, unsigned _supla_int_t pack_sizeof,
    unsigned _supla_int_t item_sizeof, unsigned _supla_int_t pack_max_count,
    unsigned _supla_int_t caption_max_size,
    _func_srpc_pack_get_pack_count pack_get_count,
    _func_srpc_pack_set_pack_count pack_set_count,
    _func_srpc_pack_get_item_ptr get_item_ptr,
    _func_srpc_pack_get_item_caption_size get_item_caption_size) {
  _supla_int_t header_size = pack_sizeof - (item_sizeof * pack_max_count);
  _supla_int_t c_header_size = item_sizeof - caption_max_size;
  _supla_int_t a, count, size, offset, pack_size;
  void *pack = NULL;

  if (srpc->sdp.data_size < header_size || srpc->sdp.data_size > pack_sizeof) {
    return;
  }

  count = pack_get_count(srpc->sdp.data);

  if (count < 0 || count > pack_max_count) {
    return;
  }

  pack_size = header_size + (item_sizeof * count);
  pack = (TSC_SuplaChannelPack *)malloc(pack_size);

  if (pack == NULL) return;

  memset(pack, 0, pack_size);
  memcpy(pack, srpc->sdp.data, header_size);

  offset = header_size;
  pack_set_count(pack, 0, 0);

  for (a = 0; a < count; a++)
    if (srpc->sdp.data_size - offset >= c_header_size) {
      size = get_item_caption_size(&srpc->sdp.data[offset]);

      if (size >= 0 && size <= caption_max_size &&
          srpc->sdp.data_size - offset >= c_header_size + size) {
        memcpy(get_item_ptr(pack, a), &srpc->sdp.data[offset],
               c_header_size + size);
        offset += c_header_size + size;
        pack_set_count(pack, 1, 1);

      } else {
        break;
      }
    }

  if (count == pack_get_count(pack)) {
    srpc->sdp.data_size = 0;
    // dcs_ping is 1st variable in union
    rd->data.dcs_ping = pack;

  } else {
    free(pack);
  }
}

void *SRPC_ICACHE_FLASH srpc_channelpack_get_item_ptr(void *pack,
                                                      _supla_int_t idx) {
  return &((TSC_SuplaChannelPack *)pack)->items[idx];  // NOLINT
}

_supla_int_t SRPC_ICACHE_FLASH srpc_channelpack_get_pack_count(void *pack) {
  return ((TSC_SuplaChannelPack *)pack)->count;
}

void SRPC_ICACHE_FLASH srpc_channelpack_set_pack_count(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaChannelPack *)pack)->count = count;
  } else {
    ((TSC_SuplaChannelPack *)pack)->count += count;
  }
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_channelpack_get_item_caption_size(void *item) {
  return ((TSC_SuplaChannel *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_getchannelpack(Tsrpc *srpc, TsrpcReceivedData *rd) {
  srpc_getpack(srpc, rd, sizeof(TSC_SuplaChannelPack), sizeof(TSC_SuplaChannel),
               SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
               &srpc_channelpack_get_pack_count,
               &srpc_channelpack_set_pack_count, &srpc_channelpack_get_item_ptr,
               &srpc_channelpack_get_item_caption_size);
}

void *SRPC_ICACHE_FLASH srpc_channelpack_get_item_ptr_b(void *pack,
                                                        _supla_int_t idx) {
  return &((TSC_SuplaChannelPack_B *)pack)->items[idx];  // NOLINT
}

_supla_int_t SRPC_ICACHE_FLASH srpc_channelpack_get_pack_count_b(void *pack) {
  return ((TSC_SuplaChannelPack_B *)pack)->count;
}

void SRPC_ICACHE_FLASH srpc_channelpack_set_pack_count_b(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaChannelPack_B *)pack)->count = count;
  } else {
    ((TSC_SuplaChannelPack_B *)pack)->count += count;
  }
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_channelpack_get_item_caption_size_b(void *item) {
  return ((TSC_SuplaChannel_B *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_getchannelpack_b(Tsrpc *srpc,
                                             TsrpcReceivedData *rd) {
  srpc_getpack(
      srpc, rd, sizeof(TSC_SuplaChannelPack_B), sizeof(TSC_SuplaChannel_B),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      &srpc_channelpack_get_pack_count_b, &srpc_channelpack_set_pack_count_b,
      &srpc_channelpack_get_item_ptr_b,
      &srpc_channelpack_get_item_caption_size_b);
}

void *SRPC_ICACHE_FLASH srpc_channelpack_get_item_ptr_c(void *pack,
                                                        _supla_int_t idx) {
  return &((TSC_SuplaChannelPack_C *)pack)->items[idx];  // NOLINT
}

_supla_int_t SRPC_ICACHE_FLASH srpc_channelpack_get_pack_count_c(void *pack) {
  return ((TSC_SuplaChannelPack_C *)pack)->count;
}

void SRPC_ICACHE_FLASH srpc_channelpack_set_pack_count_c(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaChannelPack_C *)pack)->count = count;
  } else {
    ((TSC_SuplaChannelPack_C *)pack)->count += count;
  }
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_channelpack_get_item_caption_size_c(void *item) {
  return ((TSC_SuplaChannel_C *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_getchannelpack_c(Tsrpc *srpc,
                                             TsrpcReceivedData *rd) {
  srpc_getpack(
      srpc, rd, sizeof(TSC_SuplaChannelPack_C), sizeof(TSC_SuplaChannel_C),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      &srpc_channelpack_get_pack_count_c, &srpc_channelpack_set_pack_count_c,
      &srpc_channelpack_get_item_ptr_c,
      &srpc_channelpack_get_item_caption_size_c);
}

void *SRPC_ICACHE_FLASH srpc_channelpack_get_item_ptr_d(void *pack,
                                                        _supla_int_t idx) {
  return &((TSC_SuplaChannelPack_D *)pack)->items[idx];  // NOLINT
}

_supla_int_t SRPC_ICACHE_FLASH srpc_channelpack_get_pack_count_d(void *pack) {
  return ((TSC_SuplaChannelPack_D *)pack)->count;
}

void SRPC_ICACHE_FLASH srpc_channelpack_set_pack_count_d(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaChannelPack_D *)pack)->count = count;
  } else {
    ((TSC_SuplaChannelPack_D *)pack)->count += count;
  }
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_channelpack_get_item_caption_size_d(void *item) {
  return ((TSC_SuplaChannel_D *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_getchannelpack_d(Tsrpc *srpc,
                                             TsrpcReceivedData *rd) {
  srpc_getpack(
      srpc, rd, sizeof(TSC_SuplaChannelPack_D), sizeof(TSC_SuplaChannel_D),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      &srpc_channelpack_get_pack_count_d, &srpc_channelpack_set_pack_count_d,
      &srpc_channelpack_get_item_ptr_d,
      &srpc_channelpack_get_item_caption_size_d);
}

void *SRPC_ICACHE_FLASH srpc_channelpack_get_item_ptr_e(void *pack,
                                                        _supla_int_t idx) {
  return &((TSC_SuplaChannelPack_E *)pack)->items[idx];  // NOLINT
}

_supla_int_t SRPC_ICACHE_FLASH srpc_channelpack_get_pack_count_e(void *pack) {
  return ((TSC_SuplaChannelPack_E *)pack)->count;
}

void SRPC_ICACHE_FLASH srpc_channelpack_set_pack_count_e(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaChannelPack_E *)pack)->count = count;
  } else {
    ((TSC_SuplaChannelPack_E *)pack)->count += count;
  }
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_channelpack_get_item_caption_size_e(void *item) {
  return ((TSC_SuplaChannel_E *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_getchannelpack_e(Tsrpc *srpc,
                                             TsrpcReceivedData *rd) {
  srpc_getpack(
      srpc, rd, sizeof(TSC_SuplaChannelPack_E), sizeof(TSC_SuplaChannel_E),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      &srpc_channelpack_get_pack_count_e, &srpc_channelpack_set_pack_count_e,
      &srpc_channelpack_get_item_ptr_e,
      &srpc_channelpack_get_item_caption_size_e);
}

void *SRPC_ICACHE_FLASH srpc_channelgroup_pack_get_item_ptr(void *pack,
                                                            _supla_int_t idx) {
  return &((TSC_SuplaChannelGroupPack *)pack)->items[idx];  // NOLINT
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_channelgroup_pack_get_pack_count(void *pack) {
  return ((TSC_SuplaChannelGroupPack *)pack)->count;
}

void SRPC_ICACHE_FLASH srpc_channelgroup_pack_set_pack_count(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaChannelGroupPack *)pack)->count = count;
  } else {
    ((TSC_SuplaChannelGroupPack *)pack)->count += count;
  }
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_channelgroup_pack_get_item_caption_size(void *item) {
  return ((TSC_SuplaChannelGroup *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_getchannelgroup_pack(Tsrpc *srpc,
                                                 TsrpcReceivedData *rd) {
  srpc_getpack(srpc, rd, sizeof(TSC_SuplaChannelGroupPack),
               sizeof(TSC_SuplaChannelGroup), SUPLA_CHANNELGROUP_PACK_MAXCOUNT,
               SUPLA_CHANNELGROUP_CAPTION_MAXSIZE,
               &srpc_channelgroup_pack_get_pack_count,
               &srpc_channelgroup_pack_set_pack_count,
               &srpc_channelgroup_pack_get_item_ptr,
               &srpc_channelgroup_pack_get_item_caption_size);
}

void *SRPC_ICACHE_FLASH
srpc_channelgroup_pack_b_get_item_ptr(void *pack, _supla_int_t idx) {
  return &((TSC_SuplaChannelGroupPack_B *)pack)->items[idx];  // NOLINT
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_channelgroup_pack_b_get_pack_count(void *pack) {
  return ((TSC_SuplaChannelGroupPack_B *)pack)->count;
}

void SRPC_ICACHE_FLASH srpc_channelgroup_pack_b_set_pack_count(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaChannelGroupPack_B *)pack)->count = count;
  } else {
    ((TSC_SuplaChannelGroupPack_B *)pack)->count += count;
  }
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_channelgroup_pack_b_get_item_caption_size(void *item) {
  return ((TSC_SuplaChannelGroup_B *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_getchannelgroup_pack_b(Tsrpc *srpc,
                                                   TsrpcReceivedData *rd) {
  srpc_getpack(srpc, rd, sizeof(TSC_SuplaChannelGroupPack_B),
               sizeof(TSC_SuplaChannelGroup_B),
               SUPLA_CHANNELGROUP_PACK_MAXCOUNT,
               SUPLA_CHANNELGROUP_CAPTION_MAXSIZE,
               &srpc_channelgroup_pack_b_get_pack_count,
               &srpc_channelgroup_pack_b_set_pack_count,
               &srpc_channelgroup_pack_b_get_item_ptr,
               &srpc_channelgroup_pack_b_get_item_caption_size);
}

void *SRPC_ICACHE_FLASH srpc_locationpack_get_item_ptr(void *pack,
                                                       _supla_int_t idx) {
  return &((TSC_SuplaLocationPack *)pack)->items[idx];  // NOLINT
}

_supla_int_t SRPC_ICACHE_FLASH srpc_locationpack_get_pack_count(void *pack) {
  return ((TSC_SuplaLocationPack *)pack)->count;
}

void SRPC_ICACHE_FLASH srpc_locationpack_set_pack_count(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaLocationPack *)pack)->count = count;
  } else {
    ((TSC_SuplaLocationPack *)pack)->count += count;
  }
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_locationpack_get_item_caption_size(void *item) {
  return ((TSC_SuplaLocation *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_getlocationpack(Tsrpc *srpc,
                                            TsrpcReceivedData *rd) {
  srpc_getpack(
      srpc, rd, sizeof(TSC_SuplaLocationPack), sizeof(TSC_SuplaLocation),
      SUPLA_LOCATIONPACK_MAXCOUNT, SUPLA_LOCATION_CAPTION_MAXSIZE,
      &srpc_locationpack_get_pack_count, &srpc_locationpack_set_pack_count,
      &srpc_locationpack_get_item_ptr,
      &srpc_locationpack_get_item_caption_size);
}

#define VALID_SIZE(MIAN_TYPE, ITEM_TYPE, SIZE_VAR, MAX)                   \
  srpc->sdp.data_size >= (sizeof(MIAN_TYPE) - sizeof(ITEM_TYPE) * MAX) && \
      srpc->sdp.data_size <= sizeof(MIAN_TYPE) &&                         \
      (((MIAN_TYPE *)srpc->sdp.data)->SIZE_VAR) * sizeof(ITEM_TYPE) ==    \
          srpc->sdp.data_size - (sizeof(MIAN_TYPE) - sizeof(ITEM_TYPE) * MAX)

char SRPC_ICACHE_FLASH srpc_getdata(void *_srpc, TsrpcReceivedData *rd,
                                    unsigned _supla_int_t rr_id) {
  Tsrpc *srpc = (Tsrpc *)_srpc;
  char call_with_no_data = 0;
  rd->call_id = 0;

  lck_lock(srpc->lck);

  if (SUPLA_RESULT_TRUE == srpc_in_queue_pop(srpc, &srpc->sdp, rr_id)) {
    rd->call_id = srpc->sdp.call_id;
    rd->rr_id = srpc->sdp.rr_id;

    // first one
    rd->data.dcs_ping = NULL;

    switch (srpc->sdp.call_id) {
      case SUPLA_DCS_CALL_GETVERSION:
      case SUPLA_CS_CALL_GET_NEXT:
      case SUPLA_DCS_CALL_GET_REGISTRATION_ENABLED:
        call_with_no_data = 1;
        break;

      case SUPLA_SDC_CALL_GETVERSION_RESULT:

        if (srpc->sdp.data_size == sizeof(TSDC_SuplaGetVersionResult))
          rd->data.sdc_getversion_result = (TSDC_SuplaGetVersionResult *)calloc(
              1, sizeof(TSDC_SuplaGetVersionResult));

        break;

      case SUPLA_SDC_CALL_VERSIONERROR:

        if (srpc->sdp.data_size == sizeof(TSDC_SuplaVersionError))
          rd->data.sdc_version_error = (TSDC_SuplaVersionError *)calloc(
              1, sizeof(TSDC_SuplaVersionError));

        break;

      case SUPLA_DCS_CALL_PING_SERVER:

        if (srpc->sdp.data_size == sizeof(TDCS_SuplaPingServer) ||
            srpc->sdp.data_size == sizeof(TDCS_SuplaPingServer_COMPAT)) {
          rd->data.dcs_ping =
              (TDCS_SuplaPingServer *)calloc(1, sizeof(TDCS_SuplaPingServer));

#ifndef __AVR__
          if (srpc->sdp.data_size == sizeof(TDCS_SuplaPingServer_COMPAT)) {
            TDCS_SuplaPingServer_COMPAT *compat =
                (TDCS_SuplaPingServer_COMPAT *)srpc->sdp.data;

            rd->data.dcs_ping->now.tv_sec = compat->now.tv_sec;
            rd->data.dcs_ping->now.tv_usec = compat->now.tv_usec;
            call_with_no_data = 1;
          }
#endif
        }
        break;

      case SUPLA_SDC_CALL_PING_SERVER_RESULT:

        if (srpc->sdp.data_size == sizeof(TSDC_SuplaPingServerResult))
          rd->data.sdc_ping_result = (TSDC_SuplaPingServerResult *)calloc(
              1, sizeof(TSDC_SuplaPingServerResult));

        break;

      case SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT:

        if (srpc->sdp.data_size == sizeof(TDCS_SuplaSetActivityTimeout))
          rd->data.dcs_set_activity_timeout =
              (TDCS_SuplaSetActivityTimeout *)calloc(
                  1, sizeof(TDCS_SuplaSetActivityTimeout));

        break;

      case SUPLA_SDC_CALL_SET_ACTIVITY_TIMEOUT_RESULT:

        if (srpc->sdp.data_size == sizeof(TSDC_SuplaSetActivityTimeoutResult))
          rd->data.sdc_set_activity_timeout_result =
              (TSDC_SuplaSetActivityTimeoutResult *)calloc(
                  1, sizeof(TSDC_SuplaSetActivityTimeoutResult));

        break;

      case SUPLA_SDC_CALL_GET_REGISTRATION_ENABLED_RESULT:

        if (srpc->sdp.data_size == sizeof(TSDC_RegistrationEnabled))
          rd->data.sdc_reg_enabled = (TSDC_RegistrationEnabled *)calloc(
              1, sizeof(TSDC_RegistrationEnabled));

        break;
      case SUPLA_DCS_CALL_GET_USER_LOCALTIME:
        call_with_no_data = 1;
        break;
      case SUPLA_DCS_CALL_GET_USER_LOCALTIME_RESULT:
        if (VALID_SIZE(TSDC_UserLocalTimeResult, char, timezoneSize,
                       SUPLA_TIMEZONE_MAXSIZE)) {
          rd->data.sdc_user_localtime_result =
              (TSDC_UserLocalTimeResult *)calloc(
                  1, sizeof(TSDC_UserLocalTimeResult));
        }

        break;

      case SUPLA_CSD_CALL_GET_CHANNEL_STATE:
        if (srpc->sdp.data_size == sizeof(TCSD_ChannelStateRequest))
          rd->data.csd_channel_state_request =
              (TCSD_ChannelStateRequest *)calloc(
                  1, sizeof(TCSD_ChannelStateRequest));
        break;
      case SUPLA_DSC_CALL_CHANNEL_STATE_RESULT:
        if (srpc->sdp.data_size == sizeof(TDSC_ChannelState))
          rd->data.dsc_channel_state =
              (TDSC_ChannelState *)calloc(1, sizeof(TDSC_ChannelState));
        break;

#ifndef SRPC_EXCLUDE_DEVICE
      case SUPLA_DS_CALL_REGISTER_DEVICE:
        if (VALID_SIZE(TDS_SuplaRegisterDevice, TDS_SuplaDeviceChannel,
                       channel_count, SUPLA_CHANNELMAXCOUNT)) {
          rd->data.ds_register_device = (TDS_SuplaRegisterDevice *)calloc(
              1, sizeof(TDS_SuplaRegisterDevice));
        }

        break;

      case SUPLA_DS_CALL_REGISTER_DEVICE_B:  // ver. >= 2

        if (VALID_SIZE(TDS_SuplaRegisterDevice_B, TDS_SuplaDeviceChannel_B,
                       channel_count, SUPLA_CHANNELMAXCOUNT)) {
          rd->data.ds_register_device_b = (TDS_SuplaRegisterDevice_B *)calloc(
              1, sizeof(TDS_SuplaRegisterDevice_B));
        }

        break;

      case SUPLA_DS_CALL_REGISTER_DEVICE_C:  // ver. >= 6

        if (VALID_SIZE(TDS_SuplaRegisterDevice_C, TDS_SuplaDeviceChannel_B,
                       channel_count, SUPLA_CHANNELMAXCOUNT)) {
          rd->data.ds_register_device_c = (TDS_SuplaRegisterDevice_C *)calloc(
              1, sizeof(TDS_SuplaRegisterDevice_C));
        }

        break;

      case SUPLA_DS_CALL_REGISTER_DEVICE_D:  // ver. >= 7

        if (VALID_SIZE(TDS_SuplaRegisterDevice_D, TDS_SuplaDeviceChannel_B,
                       channel_count, SUPLA_CHANNELMAXCOUNT)) {
          rd->data.ds_register_device_d = (TDS_SuplaRegisterDevice_D *)calloc(
              1, sizeof(TDS_SuplaRegisterDevice_D));
        }

        break;

      case SUPLA_DS_CALL_REGISTER_DEVICE_E:  // ver. >= 10

        if (VALID_SIZE(TDS_SuplaRegisterDevice_E, TDS_SuplaDeviceChannel_C,
                       channel_count, SUPLA_CHANNELMAXCOUNT)) {
          rd->data.ds_register_device_e = (TDS_SuplaRegisterDevice_E *)calloc(
              1, sizeof(TDS_SuplaRegisterDevice_E));
        }

        break;

      case SUPLA_DS_CALL_REGISTER_DEVICE_F:  // ver. >= 23

        if (VALID_SIZE(TDS_SuplaRegisterDevice_F, TDS_SuplaDeviceChannel_D,
                       channel_count, SUPLA_CHANNELMAXCOUNT)) {
          rd->data.ds_register_device_f = (TDS_SuplaRegisterDevice_F *)calloc(
              1, sizeof(TDS_SuplaRegisterDevice_F));
        }

        break;

      case SUPLA_SD_CALL_REGISTER_DEVICE_RESULT:

        if (srpc->sdp.data_size == sizeof(TSD_SuplaRegisterDeviceResult))
          rd->data.sd_register_device_result =
              (TSD_SuplaRegisterDeviceResult *)calloc(
                  1, sizeof(TSD_SuplaRegisterDeviceResult));
        break;

      case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED:

        if (srpc->sdp.data_size == sizeof(TDS_SuplaDeviceChannelValue))
          rd->data.ds_device_channel_value =
              (TDS_SuplaDeviceChannelValue *)calloc(
                  1, sizeof(TDS_SuplaDeviceChannelValue));

        break;

      case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B:

        if (srpc->sdp.data_size == sizeof(TDS_SuplaDeviceChannelValue_B))
          rd->data.ds_device_channel_value_b =
              (TDS_SuplaDeviceChannelValue_B *)calloc(
                  1, sizeof(TDS_SuplaDeviceChannelValue_B));

        break;

      case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C:

        if (srpc->sdp.data_size == sizeof(TDS_SuplaDeviceChannelValue_C))
          rd->data.ds_device_channel_value_c =
              (TDS_SuplaDeviceChannelValue_C *)calloc(
                  1, sizeof(TDS_SuplaDeviceChannelValue_C));

        break;

      case SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED:

        if (VALID_SIZE(TDS_SuplaDeviceChannelExtendedValue, char, value.size,
                       SUPLA_CHANNELEXTENDEDVALUE_SIZE))
          rd->data.ds_device_channel_extendedvalue =
              (TDS_SuplaDeviceChannelExtendedValue *)calloc(
                  1, sizeof(TDS_SuplaDeviceChannelExtendedValue));

        break;

      case SUPLA_SD_CALL_CHANNEL_SET_VALUE:

        if (srpc->sdp.data_size == sizeof(TSD_SuplaChannelNewValue))
          rd->data.sd_channel_new_value = (TSD_SuplaChannelNewValue *)calloc(
              1, sizeof(TSD_SuplaChannelNewValue));

        break;

      case SUPLA_SD_CALL_CHANNELGROUP_SET_VALUE:

        if (srpc->sdp.data_size == sizeof(TSD_SuplaChannelGroupNewValue))
          rd->data.sd_channelgroup_new_value =
              (TSD_SuplaChannelGroupNewValue *)calloc(
                  1, sizeof(TSD_SuplaChannelGroupNewValue));

        break;

      case SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT:

        if (srpc->sdp.data_size == sizeof(TDS_SuplaChannelNewValueResult))
          rd->data.ds_channel_new_value_result =
              (TDS_SuplaChannelNewValueResult *)calloc(
                  1, sizeof(TDS_SuplaChannelNewValueResult));

        break;

      case SUPLA_DS_CALL_GET_FIRMWARE_UPDATE_URL:

        if (srpc->sdp.data_size == sizeof(TDS_FirmwareUpdateParams))
          rd->data.ds_firmware_update_params =
              (TDS_FirmwareUpdateParams *)calloc(
                  1, sizeof(TDS_FirmwareUpdateParams));

        break;

      case SUPLA_SD_CALL_GET_FIRMWARE_UPDATE_URL_RESULT:

        if (srpc->sdp.data_size == sizeof(TSD_FirmwareUpdate_UrlResult) ||
            srpc->sdp.data_size == sizeof(char)) {
          rd->data.sc_firmware_update_url_result =
              (TSD_FirmwareUpdate_UrlResult *)calloc(
                  1, sizeof(TSD_FirmwareUpdate_UrlResult));

          if (srpc->sdp.data_size == sizeof(char) &&
              rd->data.sc_firmware_update_url_result != NULL)
            memset(rd->data.sc_firmware_update_url_result, 0,
                   sizeof(TSD_FirmwareUpdate_UrlResult));
        }
        break;
      case SUPLA_SD_CALL_DEVICE_CALCFG_REQUEST:
        if (VALID_SIZE(TSD_DeviceCalCfgRequest, char, DataSize,
                       SUPLA_CALCFG_DATA_MAXSIZE)) {
          rd->data.sd_device_calcfg_request = (TSD_DeviceCalCfgRequest *)calloc(
              1, sizeof(TSD_DeviceCalCfgRequest));
        }
        break;
      case SUPLA_DS_CALL_DEVICE_CALCFG_RESULT:
        if (VALID_SIZE(TDS_DeviceCalCfgResult, char, DataSize,
                       SUPLA_CALCFG_DATA_MAXSIZE)) {
          rd->data.ds_device_calcfg_result = (TDS_DeviceCalCfgResult *)calloc(
              1, sizeof(TDS_DeviceCalCfgResult));
        }
        break;
      case SUPLA_DS_CALL_GET_CHANNEL_FUNCTIONS:
        call_with_no_data = 1;
        break;
      case SUPLA_SD_CALL_GET_CHANNEL_FUNCTIONS_RESULT:
        if (VALID_SIZE(TSD_ChannelFunctions, _supla_int_t, ChannelCount,
                       SUPLA_CHANNELMAXCOUNT)) {
          rd->data.sd_channel_functions =
              (TSD_ChannelFunctions *)calloc(1, sizeof(TSD_ChannelFunctions));
        }
        break;
      case SUPLA_DS_CALL_GET_CHANNEL_CONFIG:
        if (srpc->sdp.data_size == sizeof(TDS_GetChannelConfigRequest)) {
          rd->data.ds_get_channel_config_request =
              (TDS_GetChannelConfigRequest *)calloc(
                  1, sizeof(TDS_GetChannelConfigRequest));
        }
        break;
      case SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT:
        if (VALID_SIZE(TSD_ChannelConfig, char, ConfigSize,
                       SUPLA_CHANNEL_CONFIG_MAXSIZE)) {
          rd->data.sd_channel_config =
              (TSD_ChannelConfig *)calloc(1, sizeof(TSD_ChannelConfig));
        }
        break;
      case SUPLA_DS_CALL_ACTIONTRIGGER:
        if (srpc->sdp.data_size == sizeof(TDS_ActionTrigger)) {
          rd->data.ds_action_trigger =
              (TDS_ActionTrigger *)calloc(1, sizeof(TDS_ActionTrigger));
        }
        break;
      case SUPLA_DS_CALL_REGISTER_PUSH_NOTIFICATION:
        if (srpc->sdp.data_size == sizeof(TDS_RegisterPushNotification)) {
          rd->data.ds_register_push_notification =
              (TDS_RegisterPushNotification *)calloc(
                  1, sizeof(TDS_RegisterPushNotification));
        }
        break;
      case SUPLA_DS_CALL_SEND_PUSH_NOTIFICATION:
        if (VALID_SIZE(
                TDS_PushNotification, char,
                TitleSize + ((TDS_PushNotification *)srpc->sdp.data)->BodySize,
                (SUPLA_PN_TITLE_MAXSIZE + SUPLA_PN_BODY_MAXSIZE))) {
          rd->data.ds_push_notification =
              (TDS_PushNotification *)calloc(1, sizeof(TDS_PushNotification));
        }
        break;
      case SUPLA_DS_CALL_SET_CHANNEL_CONFIG:
      case SUPLA_SD_CALL_SET_CHANNEL_CONFIG:
        if (VALID_SIZE(TSDS_SetChannelConfig, char, ConfigSize,
                       SUPLA_CHANNEL_CONFIG_MAXSIZE)) {
          rd->data.sds_set_channel_config_request =
              (TSDS_SetChannelConfig *)malloc(sizeof(TSDS_SetChannelConfig));
        }
        break;
      case SUPLA_DS_CALL_SET_CHANNEL_CONFIG_RESULT:
      case SUPLA_SD_CALL_SET_CHANNEL_CONFIG_RESULT:
        if (srpc->sdp.data_size == sizeof(TSDS_SetChannelConfigResult)) {
          rd->data.sds_set_channel_config_result =
              (TSDS_SetChannelConfigResult *)malloc(
                  sizeof(TSDS_SetChannelConfigResult));
        }
        break;
      case SUPLA_SD_CALL_CHANNEL_CONFIG_FINISHED:
        if (srpc->sdp.data_size == sizeof(TSD_ChannelConfigFinished)) {
          rd->data.sd_channel_config_finished =
              (TSD_ChannelConfigFinished *)malloc(
                  sizeof(TSD_ChannelConfigFinished));
        }
        break;
      case SUPLA_DS_CALL_SET_DEVICE_CONFIG:
      case SUPLA_SD_CALL_SET_DEVICE_CONFIG:
        if (VALID_SIZE(TSDS_SetDeviceConfig, char, ConfigSize,
                       SUPLA_DEVICE_CONFIG_MAXSIZE)) {
          rd->data.sds_set_device_config_request =
              (TSDS_SetDeviceConfig *)malloc(sizeof(TSDS_SetDeviceConfig));
        }
        break;
      case SUPLA_SD_CALL_SET_DEVICE_CONFIG_RESULT:
      case SUPLA_DS_CALL_SET_DEVICE_CONFIG_RESULT:
        if (srpc->sdp.data_size == sizeof(TSDS_SetDeviceConfigResult)) {
          rd->data.sds_set_device_config_result =
              (TSDS_SetDeviceConfigResult *)malloc(
                  sizeof(TSDS_SetDeviceConfigResult));
        }
        break;
#endif /*#ifndef SRPC_EXCLUDE_DEVICE*/

#ifndef SRPC_EXCLUDE_CLIENT
      case SUPLA_CS_CALL_REGISTER_CLIENT:

        if (srpc->sdp.data_size == sizeof(TCS_SuplaRegisterClient))
          rd->data.cs_register_client = (TCS_SuplaRegisterClient *)calloc(
              1, sizeof(TCS_SuplaRegisterClient));

        break;

      case SUPLA_CS_CALL_REGISTER_CLIENT_B:  // ver. >= 6

        if (srpc->sdp.data_size == sizeof(TCS_SuplaRegisterClient_B))
          rd->data.cs_register_client_b = (TCS_SuplaRegisterClient_B *)calloc(
              1, sizeof(TCS_SuplaRegisterClient_B));

        break;

      case SUPLA_CS_CALL_REGISTER_CLIENT_C:  // ver. >= 7

        if (srpc->sdp.data_size == sizeof(TCS_SuplaRegisterClient_C))
          rd->data.cs_register_client_c = (TCS_SuplaRegisterClient_C *)calloc(
              1, sizeof(TCS_SuplaRegisterClient_C));

        break;

      case SUPLA_CS_CALL_REGISTER_CLIENT_D:  // ver. >= 12

        if (srpc->sdp.data_size == sizeof(TCS_SuplaRegisterClient_D))
          rd->data.cs_register_client_d = (TCS_SuplaRegisterClient_D *)calloc(
              1, sizeof(TCS_SuplaRegisterClient_D));

        break;

      case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT:

        if (srpc->sdp.data_size == sizeof(TSC_SuplaRegisterClientResult))
          rd->data.sc_register_client_result =
              (TSC_SuplaRegisterClientResult *)calloc(
                  1, sizeof(TSC_SuplaRegisterClientResult));

        break;

      case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B:

        if (srpc->sdp.data_size == sizeof(TSC_SuplaRegisterClientResult_B))
          rd->data.sc_register_client_result_b =
              (TSC_SuplaRegisterClientResult_B *)calloc(
                  1, sizeof(TSC_SuplaRegisterClientResult_B));

        break;

      case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_C:

        if (srpc->sdp.data_size == sizeof(TSC_SuplaRegisterClientResult_C))
          rd->data.sc_register_client_result_c =
              (TSC_SuplaRegisterClientResult_C *)calloc(
                  1, sizeof(TSC_SuplaRegisterClientResult_C));

        break;

      case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_D:

        if (srpc->sdp.data_size == sizeof(TSC_SuplaRegisterClientResult_D))
          rd->data.sc_register_client_result_d =
              (TSC_SuplaRegisterClientResult_D *)calloc(
                  1, sizeof(TSC_SuplaRegisterClientResult_D));

        break;

      case SUPLA_SC_CALL_LOCATION_UPDATE:

        if (VALID_SIZE(TSC_SuplaLocation, char, CaptionSize,
                       SUPLA_LOCATION_CAPTION_MAXSIZE)) {
          rd->data.sc_location =
              (TSC_SuplaLocation *)calloc(1, sizeof(TSC_SuplaLocation));
        }

        break;

      case SUPLA_SC_CALL_LOCATIONPACK_UPDATE:
        srpc_getlocationpack(srpc, rd);
        break;

      case SUPLA_SC_CALL_CHANNELPACK_UPDATE:
        srpc_getchannelpack(srpc, rd);
        break;

      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_B:
        srpc_getchannelpack_b(srpc, rd);
        break;

      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_C:
        srpc_getchannelpack_c(srpc, rd);
        break;

      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_D:
        srpc_getchannelpack_d(srpc, rd);
        break;

      case SUPLA_SC_CALL_CHANNELPACK_UPDATE_E:
        srpc_getchannelpack_e(srpc, rd);
        break;

      case SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE:

        if (srpc->sdp.data_size == sizeof(TSC_SuplaChannelValue))
          rd->data.sc_channel_value =
              (TSC_SuplaChannelValue *)calloc(1, sizeof(TSC_SuplaChannelValue));

        break;

      case SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE_B:

        if (srpc->sdp.data_size == sizeof(TSC_SuplaChannelValue_B))
          rd->data.sc_channel_value_b = (TSC_SuplaChannelValue_B *)calloc(
              1, sizeof(TSC_SuplaChannelValue_B));

        break;

      case SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE:
        srpc_getchannelgroup_pack(srpc, rd);
        break;

      case SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B:
        srpc_getchannelgroup_pack_b(srpc, rd);
        break;

      case SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE:
        if (VALID_SIZE(TSC_SuplaChannelGroupRelationPack,
                       TSC_SuplaChannelGroupRelation, count,
                       SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT)) {
          rd->data.sc_channelgroup_relation_pack =
              (TSC_SuplaChannelGroupRelationPack *)calloc(
                  1, sizeof(TSC_SuplaChannelGroupRelationPack));
        }
        break;

      case SUPLA_SC_CALL_CHANNEL_RELATION_PACK_UPDATE:
        if (VALID_SIZE(TSC_SuplaChannelRelationPack, TSC_SuplaChannelRelation,
                       count, SUPLA_CHANNEL_RELATION_PACK_MAXCOUNT)) {
          rd->data.sc_channel_relation_pack =
              (TSC_SuplaChannelRelationPack *)calloc(
                  1, sizeof(TSC_SuplaChannelRelationPack));
        }
        break;

      case SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE:
        if (VALID_SIZE(TSC_SuplaChannelValuePack, TSC_SuplaChannelValue, count,
                       SUPLA_CHANNELVALUE_PACK_MAXCOUNT) &&
            srpc->sdp.data_size <= sizeof(TSC_SuplaChannelValuePack)) {
          rd->data.sc_channelvalue_pack = (TSC_SuplaChannelValuePack *)calloc(
              1, sizeof(TSC_SuplaChannelValuePack));
        }
        break;

      case SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE_B:
        if (VALID_SIZE(TSC_SuplaChannelValuePack_B, TSC_SuplaChannelValue_B,
                       count, SUPLA_CHANNELVALUE_PACK_MAXCOUNT)) {
          rd->data.sc_channelvalue_pack_b =
              (TSC_SuplaChannelValuePack_B *)calloc(
                  1, sizeof(TSC_SuplaChannelValuePack_B));
        }
        break;

      case SUPLA_SC_CALL_CHANNELEXTENDEDVALUE_PACK_UPDATE:
        if (VALID_SIZE(TSC_SuplaChannelExtendedValuePack, char, pack_size,
                       SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE)) {
          rd->data.sc_channelextendedvalue_pack =
              (TSC_SuplaChannelExtendedValuePack *)calloc(
                  1, sizeof(TSC_SuplaChannelExtendedValuePack));
        }
        break;

      case SUPLA_CS_CALL_CHANNEL_SET_VALUE:

        if (srpc->sdp.data_size == sizeof(TCS_SuplaChannelNewValue))
          rd->data.cs_channel_new_value = (TCS_SuplaChannelNewValue *)calloc(
              1, sizeof(TCS_SuplaChannelNewValue));

        break;

      case SUPLA_CS_CALL_SET_VALUE:

        if (srpc->sdp.data_size == sizeof(TCS_SuplaNewValue))
          rd->data.cs_new_value =
              (TCS_SuplaNewValue *)calloc(1, sizeof(TCS_SuplaNewValue));

        break;

      case SUPLA_CS_CALL_CHANNEL_SET_VALUE_B:

        if (srpc->sdp.data_size == sizeof(TCS_SuplaChannelNewValue_B))
          rd->data.cs_channel_new_value_b =
              (TCS_SuplaChannelNewValue_B *)calloc(
                  1, sizeof(TCS_SuplaChannelNewValue_B));

        break;

      case SUPLA_SC_CALL_EVENT:

        if (VALID_SIZE(TSC_SuplaEvent, char, SenderNameSize,
                       SUPLA_SENDER_NAME_MAXSIZE)) {
          rd->data.sc_event =
              (TSC_SuplaEvent *)calloc(1, sizeof(TSC_SuplaEvent));
        }

        break;

      case SUPLA_CS_CALL_OAUTH_TOKEN_REQUEST:
        call_with_no_data = 1;
        break;

      case SUPLA_SC_CALL_OAUTH_TOKEN_REQUEST_RESULT:
        if (VALID_SIZE(TSC_OAuthTokenRequestResult, char, Token.TokenSize,
                       SUPLA_OAUTH_TOKEN_MAXSIZE)) {
          rd->data.sc_oauth_tokenrequest_result =
              (TSC_OAuthTokenRequestResult *)calloc(
                  1, sizeof(TSC_OAuthTokenRequestResult));
        }
        break;
      case SUPLA_CS_CALL_SUPERUSER_AUTHORIZATION_REQUEST:
        if (srpc->sdp.data_size == sizeof(TCS_SuperUserAuthorizationRequest))
          rd->data.cs_superuser_authorization_request =
              (TCS_SuperUserAuthorizationRequest *)calloc(
                  1, sizeof(TCS_SuperUserAuthorizationRequest));
        break;
      case SUPLA_CS_CALL_GET_SUPERUSER_AUTHORIZATION_RESULT:
        call_with_no_data = 1;
        break;
      case SUPLA_SC_CALL_SUPERUSER_AUTHORIZATION_RESULT:
        if (srpc->sdp.data_size == sizeof(TSC_SuperUserAuthorizationResult))
          rd->data.sc_superuser_authorization_result =
              (TSC_SuperUserAuthorizationResult *)calloc(
                  1, sizeof(TSC_SuperUserAuthorizationResult));
        break;
      case SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST:
        if (VALID_SIZE(TCS_DeviceCalCfgRequest, char, DataSize,
                       SUPLA_CALCFG_DATA_MAXSIZE)) {
          rd->data.cs_device_calcfg_request = (TCS_DeviceCalCfgRequest *)calloc(
              1, sizeof(TCS_DeviceCalCfgRequest));
        }
        break;
      case SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B:
        if (VALID_SIZE(TCS_DeviceCalCfgRequest_B, char, DataSize,
                       SUPLA_CALCFG_DATA_MAXSIZE)) {
          rd->data.cs_device_calcfg_request_b =
              (TCS_DeviceCalCfgRequest_B *)calloc(
                  1, sizeof(TCS_DeviceCalCfgRequest_B));
        }
        break;
      case SUPLA_SC_CALL_DEVICE_CALCFG_RESULT:
        if (VALID_SIZE(TSC_DeviceCalCfgResult, char, DataSize,
                       SUPLA_CALCFG_DATA_MAXSIZE)) {
          rd->data.sc_device_calcfg_result = (TSC_DeviceCalCfgResult *)calloc(
              1, sizeof(TSC_DeviceCalCfgResult));
        }
        break;

      case SUPLA_CS_CALL_GET_CHANNEL_BASIC_CFG:
        if (srpc->sdp.data_size == sizeof(TCS_ChannelBasicCfgRequest))
          rd->data.cs_channel_basic_cfg_request =
              (TCS_ChannelBasicCfgRequest *)calloc(
                  1, sizeof(TCS_ChannelBasicCfgRequest));
        break;
      case SUPLA_SC_CALL_CHANNEL_BASIC_CFG_RESULT:
        if (VALID_SIZE(TSC_ChannelBasicCfg, char, CaptionSize,
                       SUPLA_CHANNEL_CAPTION_MAXSIZE))
          rd->data.sc_channel_basic_cfg =
              (TSC_ChannelBasicCfg *)calloc(1, sizeof(TSC_ChannelBasicCfg));
        break;

      case SUPLA_CS_CALL_SET_CHANNEL_FUNCTION:
        if (srpc->sdp.data_size == sizeof(TCS_SetChannelFunction))
          rd->data.cs_set_channel_function = (TCS_SetChannelFunction *)calloc(
              1, sizeof(TCS_SetChannelFunction));
        break;

      case SUPLA_SC_CALL_SET_CHANNEL_FUNCTION_RESULT:
        if (srpc->sdp.data_size == sizeof(TSC_SetChannelFunctionResult))
          rd->data.sc_set_channel_function_result =
              (TSC_SetChannelFunctionResult *)calloc(
                  1, sizeof(TSC_SetChannelFunctionResult));
        break;

      case SUPLA_CS_CALL_SET_CHANNEL_CAPTION:
      case SUPLA_CS_CALL_SET_CHANNEL_GROUP_CAPTION:
      case SUPLA_CS_CALL_SET_LOCATION_CAPTION:
      case SUPLA_CS_CALL_SET_SCENE_CAPTION:
        if (VALID_SIZE(TCS_SetCaption, char, CaptionSize,
                       SUPLA_CAPTION_MAXSIZE))
          rd->data.cs_set_caption =
              (TCS_SetCaption *)calloc(1, sizeof(TCS_SetCaption));
        break;

      case SUPLA_SC_CALL_SET_CHANNEL_CAPTION_RESULT:
      case SUPLA_SC_CALL_SET_CHANNEL_GROUP_CAPTION_RESULT:
      case SUPLA_SC_CALL_SET_LOCATION_CAPTION_RESULT:
      case SUPLA_SC_CALL_SET_SCENE_CAPTION_RESULT:
        if (VALID_SIZE(TSC_SetCaptionResult, char, CaptionSize,
                       SUPLA_CAPTION_MAXSIZE))
          rd->data.sc_set_caption_result =
              (TSC_SetCaptionResult *)calloc(1, sizeof(TSC_SetCaptionResult));
        break;

      case SUPLA_CS_CALL_CLIENTS_RECONNECT_REQUEST:
        call_with_no_data = 1;
        break;

      case SUPLA_SC_CALL_CLIENTS_RECONNECT_REQUEST_RESULT:
        if (srpc->sdp.data_size == sizeof(TSC_ClientsReconnectRequestResult))
          rd->data.sc_clients_reconnect_result =
              (TSC_ClientsReconnectRequestResult *)calloc(
                  1, sizeof(TSC_ClientsReconnectRequestResult));
        break;

      case SUPLA_CS_CALL_SET_REGISTRATION_ENABLED:
        if (srpc->sdp.data_size == sizeof(TCS_SetRegistrationEnabled))
          rd->data.cs_set_registration_enabled =
              (TCS_SetRegistrationEnabled *)calloc(
                  1, sizeof(TCS_SetRegistrationEnabled));
        break;

      case SUPLA_SC_CALL_SET_REGISTRATION_ENABLED_RESULT:
        if (srpc->sdp.data_size == sizeof(TSC_SetRegistrationEnabledResult))
          rd->data.sc_set_registration_enabled_result =
              (TSC_SetRegistrationEnabledResult *)calloc(
                  1, sizeof(TSC_SetRegistrationEnabledResult));
        break;

      case SUPLA_CS_CALL_DEVICE_RECONNECT_REQUEST:
        if (srpc->sdp.data_size == sizeof(TCS_DeviceReconnectRequest))
          rd->data.cs_device_reconnect_request =
              (TCS_DeviceReconnectRequest *)calloc(
                  1, sizeof(TCS_DeviceReconnectRequest));
        break;
      case SUPLA_SC_CALL_DEVICE_RECONNECT_REQUEST_RESULT:
        if (srpc->sdp.data_size == sizeof(TSC_DeviceReconnectRequestResult))
          rd->data.sc_device_reconnect_request_result =
              (TSC_DeviceReconnectRequestResult *)calloc(
                  1, sizeof(TSC_DeviceReconnectRequestResult));
        break;

      case SUPLA_CS_CALL_TIMER_ARM:
        if (srpc->sdp.data_size == sizeof(TCS_TimerArmRequest))
          rd->data.cs_timer_arm_request =
              (TCS_TimerArmRequest *)calloc(1, sizeof(TCS_TimerArmRequest));
        break;

      case SUPLA_SC_CALL_SCENE_PACK_UPDATE:
        srpc_get_scene_pack(srpc, rd);
        break;

      case SUPLA_SC_CALL_SCENE_STATE_PACK_UPDATE:
        srpc_get_scene_state_pack(srpc, rd);
        break;

      case SUPLA_CS_CALL_EXECUTE_ACTION:
        if (VALID_SIZE(TCS_Action, char, ParamSize,
                       SUPLA_ACTION_PARAM_MAXSIZE)) {
          rd->data.cs_action = (TCS_Action *)calloc(1, sizeof(TCS_Action));
        }
        break;

      case SUPLA_CS_CALL_EXECUTE_ACTION_WITH_AUTH:
        if (VALID_SIZE(TCS_ActionWithAuth, char, Action.ParamSize,
                       SUPLA_ACTION_PARAM_MAXSIZE)) {
          rd->data.cs_action_with_auth =
              (TCS_ActionWithAuth *)calloc(1, sizeof(TCS_ActionWithAuth));
        }
        break;

      case SUPLA_SC_CALL_ACTION_EXECUTION_RESULT:
        if (srpc->sdp.data_size == sizeof(TSC_ActionExecutionResult))
          rd->data.sc_action_execution_result =
              (TSC_ActionExecutionResult *)calloc(
                  1, sizeof(TSC_ActionExecutionResult));
        break;

      case SUPLA_CS_CALL_GET_CHANNEL_VALUE_WITH_AUTH:
        if (srpc->sdp.data_size == sizeof(TCS_GetChannelValueWithAuth)) {
          rd->data.cs_get_value_with_auth =
              (TCS_GetChannelValueWithAuth *)calloc(
                  1, sizeof(TCS_GetChannelValueWithAuth));
        }
        break;

      case SUPLA_SC_CALL_GET_CHANNEL_VALUE_RESULT:
        if (VALID_SIZE(TSC_GetChannelValueResult, char, ExtendedValue.size,
                       SUPLA_CHANNELEXTENDEDVALUE_SIZE)) {
          rd->data.sc_get_value_result = (TSC_GetChannelValueResult *)calloc(
              1, sizeof(TSC_GetChannelValueResult));
        }
        break;
      case SUPLA_CS_CALL_REGISTER_PN_CLIENT_TOKEN:
        if (VALID_SIZE(TCS_RegisterPnClientToken, char, Token.TokenSize,
                       SUPLA_PN_CLIENT_TOKEN_MAXSIZE)) {
          rd->data.cs_register_pn_client_token =
              (TCS_RegisterPnClientToken *)calloc(
                  1, sizeof(TCS_RegisterPnClientToken));
        }
        break;
      case SUPLA_SC_CALL_REGISTER_PN_CLIENT_TOKEN_RESULT:
        if (srpc->sdp.data_size == sizeof(TSC_RegisterPnClientTokenResult)) {
          rd->data.sc_register_pn_client_token_result =
              (TSC_RegisterPnClientTokenResult *)calloc(
                  1, sizeof(TSC_RegisterPnClientTokenResult));
        }
        break;
      case SUPLA_CS_CALL_SET_CHANNEL_CONFIG:
        if (VALID_SIZE(TSCS_ChannelConfig, char, ConfigSize,
                       SUPLA_CHANNEL_CONFIG_MAXSIZE)) {
          rd->data.scs_channel_config =
              (TSCS_ChannelConfig *)malloc(sizeof(TSCS_ChannelConfig));
        }
        break;
      case SUPLA_CS_CALL_GET_CHANNEL_CONFIG:
        if (srpc->sdp.data_size == sizeof(TCS_GetChannelConfigRequest)) {
          rd->data.cs_get_channel_config_request =
              (TCS_GetChannelConfigRequest *)calloc(
                  1, sizeof(TCS_GetChannelConfigRequest));
        }
        break;
      case SUPLA_SC_CALL_CHANNEL_CONFIG_UPDATE_OR_RESULT:
        if (VALID_SIZE(TSC_ChannelConfigUpdateOrResult, char, Config.ConfigSize,
                       SUPLA_CHANNEL_CONFIG_MAXSIZE)) {
          rd->data.sc_channel_config_update_or_result =
              (TSC_ChannelConfigUpdateOrResult *)calloc(
                  1, sizeof(TSC_ChannelConfigUpdateOrResult));
        }
        break;
      case SUPLA_CS_CALL_GET_DEVICE_CONFIG:
        if (srpc->sdp.data_size == sizeof(TCS_GetDeviceConfigRequest)) {
          rd->data.cs_get_device_config_request =
              (TCS_GetDeviceConfigRequest *)malloc(
                  sizeof(TCS_GetDeviceConfigRequest));
        }
        break;
      case SUPLA_SC_CALL_DEVICE_CONFIG_UPDATE_OR_RESULT:
        if (VALID_SIZE(TSC_DeviceConfigUpdateOrResult, char, Config.ConfigSize,
                       SUPLA_DEVICE_CONFIG_MAXSIZE)) {
          rd->data.sc_device_config_update_or_result =
              (TSC_DeviceConfigUpdateOrResult *)malloc(
                  sizeof(TSC_DeviceConfigUpdateOrResult));
        }
        break;
#endif /*#ifndef SRPC_EXCLUDE_CLIENT*/
    }

    if (call_with_no_data == 1) {
      return lck_unlock_r(srpc->lck, SUPLA_RESULT_TRUE);
    }

    if (rd->data.dcs_ping != NULL) {
      if (srpc->sdp.data_size > 0) {
        memcpy(rd->data.dcs_ping, srpc->sdp.data, srpc->sdp.data_size);
      }

      return lck_unlock_r(srpc->lck, SUPLA_RESULT_TRUE);
    }

    return lck_unlock_r(srpc->lck, SUPLA_RESULT_DATA_ERROR);
  }

  return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
}

void SRPC_ICACHE_FLASH srpc_rd_free(TsrpcReceivedData *rd) {
  if (rd->call_id > 0) {
    // first one

    if (rd->data.dcs_ping != NULL) free(rd->data.dcs_ping);

    rd->call_id = 0;
  }
}

unsigned char SRPC_ICACHE_FLASH
srpc_call_min_version_required(void *_srpc, unsigned _supla_int_t call_id) {
  (void)(_srpc);
  switch (call_id) {
    case SUPLA_DCS_CALL_GETVERSION:
    case SUPLA_SDC_CALL_GETVERSION_RESULT:
    case SUPLA_SDC_CALL_VERSIONERROR:
    case SUPLA_DCS_CALL_PING_SERVER:
    case SUPLA_SDC_CALL_PING_SERVER_RESULT:
    case SUPLA_DS_CALL_REGISTER_DEVICE:
    case SUPLA_SD_CALL_REGISTER_DEVICE_RESULT:
    case SUPLA_CS_CALL_REGISTER_CLIENT:
    case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT:
    case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED:
    case SUPLA_SD_CALL_CHANNEL_SET_VALUE:
    case SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT:
    case SUPLA_SC_CALL_LOCATION_UPDATE:
    case SUPLA_SC_CALL_LOCATIONPACK_UPDATE:
    case SUPLA_SC_CALL_CHANNELPACK_UPDATE:
    case SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE:
    case SUPLA_CS_CALL_GET_NEXT:
    case SUPLA_SC_CALL_EVENT:
    case SUPLA_CS_CALL_CHANNEL_SET_VALUE:
      return 1;

    case SUPLA_DS_CALL_REGISTER_DEVICE_B:
    case SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT:
    case SUPLA_SDC_CALL_SET_ACTIVITY_TIMEOUT_RESULT:
      return 2;

    case SUPLA_CS_CALL_CHANNEL_SET_VALUE_B:
      return 3;

    case SUPLA_DS_CALL_GET_FIRMWARE_UPDATE_URL:
    case SUPLA_SD_CALL_GET_FIRMWARE_UPDATE_URL_RESULT:
      return 5;

    case SUPLA_DS_CALL_REGISTER_DEVICE_C:
    case SUPLA_CS_CALL_REGISTER_CLIENT_B:
      return 6;

    case SUPLA_CS_CALL_REGISTER_CLIENT_C:
    case SUPLA_DS_CALL_REGISTER_DEVICE_D:
    case SUPLA_DCS_CALL_GET_REGISTRATION_ENABLED:
    case SUPLA_SDC_CALL_GET_REGISTRATION_ENABLED_RESULT:
      return 7;

    case SUPLA_SC_CALL_CHANNELPACK_UPDATE_B:
      return 8;

    case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B:
    case SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE:
    case SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE:
    case SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE:
    case SUPLA_CS_CALL_SET_VALUE:
      return 9;

    case SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED:
    case SUPLA_SC_CALL_CHANNELEXTENDEDVALUE_PACK_UPDATE:
    case SUPLA_CS_CALL_OAUTH_TOKEN_REQUEST:
    case SUPLA_SC_CALL_OAUTH_TOKEN_REQUEST_RESULT:
    case SUPLA_DS_CALL_REGISTER_DEVICE_E:
    case SUPLA_CS_CALL_SUPERUSER_AUTHORIZATION_REQUEST:
    case SUPLA_SC_CALL_SUPERUSER_AUTHORIZATION_RESULT:
    case SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST:
    case SUPLA_SC_CALL_DEVICE_CALCFG_RESULT:
    case SUPLA_SD_CALL_DEVICE_CALCFG_REQUEST:
    case SUPLA_DS_CALL_DEVICE_CALCFG_RESULT:
    case SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B:
    case SUPLA_SC_CALL_CHANNELPACK_UPDATE_C:
      return 10;
    case SUPLA_DCS_CALL_GET_USER_LOCALTIME:
    case SUPLA_DCS_CALL_GET_USER_LOCALTIME_RESULT:
    case SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B:
      return 11;
    case SUPLA_CS_CALL_REGISTER_CLIENT_D:
    case SUPLA_CSD_CALL_GET_CHANNEL_STATE:
    case SUPLA_DSC_CALL_CHANNEL_STATE_RESULT:
    case SUPLA_CS_CALL_GET_CHANNEL_BASIC_CFG:
    case SUPLA_SC_CALL_CHANNEL_BASIC_CFG_RESULT:
    case SUPLA_CS_CALL_SET_CHANNEL_FUNCTION:
    case SUPLA_SC_CALL_SET_CHANNEL_FUNCTION_RESULT:
    case SUPLA_CS_CALL_SET_CHANNEL_CAPTION:
    case SUPLA_SC_CALL_SET_CHANNEL_CAPTION_RESULT:
    case SUPLA_CS_CALL_CLIENTS_RECONNECT_REQUEST:
    case SUPLA_SC_CALL_CLIENTS_RECONNECT_REQUEST_RESULT:
    case SUPLA_CS_CALL_SET_REGISTRATION_ENABLED:
    case SUPLA_SC_CALL_SET_REGISTRATION_ENABLED_RESULT:
    case SUPLA_CS_CALL_DEVICE_RECONNECT_REQUEST:
    case SUPLA_SC_CALL_DEVICE_RECONNECT_REQUEST_RESULT:
    case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B:
    case SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C:
    case SUPLA_DS_CALL_GET_CHANNEL_FUNCTIONS:
    case SUPLA_SD_CALL_GET_CHANNEL_FUNCTIONS_RESULT:
    case SUPLA_CS_CALL_GET_SUPERUSER_AUTHORIZATION_RESULT:
      return 12;
    case SUPLA_SD_CALL_CHANNELGROUP_SET_VALUE:
      return 13;
    case SUPLA_CS_CALL_SET_LOCATION_CAPTION:
    case SUPLA_SC_CALL_SET_LOCATION_CAPTION_RESULT:
      return 14;
    case SUPLA_SC_CALL_CHANNELPACK_UPDATE_D:
    case SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE_B:
    case SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE_B:
      return 15;
    case SUPLA_DS_CALL_GET_CHANNEL_CONFIG:
    case SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT:
    case SUPLA_DS_CALL_ACTIONTRIGGER:
      return 16;
    case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_C:
    case SUPLA_CS_CALL_TIMER_ARM:
      return 17;
    case SUPLA_SC_CALL_SCENE_PACK_UPDATE:
    case SUPLA_SC_CALL_SCENE_STATE_PACK_UPDATE:
      return 18;
    case SUPLA_CS_CALL_EXECUTE_ACTION:
    case SUPLA_CS_CALL_EXECUTE_ACTION_WITH_AUTH:
    case SUPLA_SC_CALL_ACTION_EXECUTION_RESULT:
    case SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_D:
    case SUPLA_CS_CALL_GET_CHANNEL_VALUE_WITH_AUTH:
    case SUPLA_SC_CALL_GET_CHANNEL_VALUE_RESULT:
    case SUPLA_CS_CALL_SET_SCENE_CAPTION:
    case SUPLA_SC_CALL_SET_SCENE_CAPTION_RESULT:
      return 19;
    case SUPLA_DS_CALL_REGISTER_PUSH_NOTIFICATION:
    case SUPLA_DS_CALL_SEND_PUSH_NOTIFICATION:
    case SUPLA_CS_CALL_REGISTER_PN_CLIENT_TOKEN:
    case SUPLA_SC_CALL_REGISTER_PN_CLIENT_TOKEN_RESULT:
    case SUPLA_CS_CALL_SET_CHANNEL_GROUP_CAPTION:
    case SUPLA_SC_CALL_SET_CHANNEL_GROUP_CAPTION_RESULT:
      return 20;
    case SUPLA_DS_CALL_SET_CHANNEL_CONFIG:
    case SUPLA_SD_CALL_SET_CHANNEL_CONFIG_RESULT:
    case SUPLA_SD_CALL_SET_CHANNEL_CONFIG:
    case SUPLA_DS_CALL_SET_CHANNEL_CONFIG_RESULT:
    case SUPLA_DS_CALL_SET_DEVICE_CONFIG:
    case SUPLA_SD_CALL_SET_DEVICE_CONFIG_RESULT:
    case SUPLA_SD_CALL_SET_DEVICE_CONFIG:
    case SUPLA_DS_CALL_SET_DEVICE_CONFIG_RESULT:
    case SUPLA_SC_CALL_CHANNEL_RELATION_PACK_UPDATE:
    case SUPLA_CS_CALL_GET_CHANNEL_CONFIG:
    case SUPLA_SC_CALL_CHANNEL_CONFIG_UPDATE_OR_RESULT:
    case SUPLA_CS_CALL_SET_CHANNEL_CONFIG:
    case SUPLA_CS_CALL_GET_DEVICE_CONFIG:
    case SUPLA_SC_CALL_DEVICE_CONFIG_UPDATE_OR_RESULT:
    case SUPLA_SD_CALL_CHANNEL_CONFIG_FINISHED:
      return 21;
    case SUPLA_SC_CALL_CHANNELPACK_UPDATE_E:
    case SUPLA_DS_CALL_REGISTER_DEVICE_F:
      return 23;
  }

  return 255;
}

unsigned char SRPC_ICACHE_FLASH
srpc_call_allowed(void *_srpc, unsigned _supla_int_t call_id) {
  unsigned char min_ver = srpc_call_min_version_required(_srpc, call_id);

  if (min_ver == 0 || srpc_get_proto_version(_srpc) >= min_ver) {
    return 1;
  }

  return 0;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_async__call(void *_srpc,
                                                unsigned _supla_int_t call_id,
                                                char *data,
                                                unsigned _supla_int_t data_size,
                                                unsigned char *version) {
  if (_srpc == NULL) {
    return SUPLA_RESULT_FALSE;
  }

  Tsrpc *srpc = (Tsrpc *)_srpc;

  if (!srpc_call_allowed(_srpc, call_id)) {
    if (srpc->params.on_min_version_required != NULL) {
      srpc->params.on_min_version_required(
          _srpc, call_id, srpc_call_min_version_required(_srpc, call_id),
          srpc->params.user_params);
    }

    return SUPLA_RESULT_FALSE;
  }

  if (srpc->params.before_async_call != NULL) {
    srpc->params.before_async_call(_srpc, call_id, srpc->params.user_params);
  }

  lck_lock(srpc->lck);

  sproto_sdp_init(srpc->proto, &srpc->sdp);

  if (version != NULL) srpc->sdp.version = *version;

  if (SUPLA_RESULT_TRUE ==
          sproto_set_data(&srpc->sdp, data, data_size, call_id) &&
      srpc_out_queue_push(srpc, &srpc->sdp)) {
#ifndef __EH_DISABLED
    if (srpc->params.eh != 0) {
      eh_raise_event(srpc->params.eh);
    }
#endif

    return lck_unlock_r(srpc->lck, srpc->sdp.rr_id);
  }

  return lck_unlock_r(srpc->lck, SUPLA_RESULT_FALSE);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_async_call(void *_srpc, unsigned _supla_int_t call_id, char *data,
                unsigned _supla_int_t data_size) {
  return srpc_async__call(_srpc, call_id, data, data_size, NULL);
}

unsigned char SRPC_ICACHE_FLASH srpc_get_proto_version(void *_srpc) {
  unsigned char version;

  if (_srpc == NULL) {
    return 0;
  }

  Tsrpc *srpc = (Tsrpc *)_srpc;
  lck_lock(srpc->lck);
  version = sproto_get_version(srpc->proto);
  lck_unlock(srpc->lck);

  return version;
}

void SRPC_ICACHE_FLASH srpc_set_proto_version(void *_srpc,
                                              unsigned char version) {
  Tsrpc *srpc = (Tsrpc *)_srpc;

  lck_lock(srpc->lck);
  sproto_set_version(srpc->proto, version);
  lck_unlock(srpc->lck);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_getversion(void *_srpc) {
  return srpc_async_call(_srpc, SUPLA_DCS_CALL_GETVERSION, NULL, 0);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sdc_async_getversion_result(
    void *_srpc, char SoftVer[SUPLA_SOFTVER_MAXSIZE]) {
  TSDC_SuplaGetVersionResult gvr;

  gvr.proto_version = SUPLA_PROTO_VERSION;
  gvr.proto_version_min = SUPLA_PROTO_VERSION_MIN;

  memcpy(gvr.SoftVer, SoftVer, SUPLA_SOFTVER_MAXSIZE);

  return srpc_async_call(_srpc, SUPLA_SDC_CALL_GETVERSION_RESULT, (char *)&gvr,
                         sizeof(TSDC_SuplaGetVersionResult));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_sdc_async_versionerror(void *_srpc, unsigned char remote_version) {
  TSDC_SuplaVersionError ve;
  ve.server_version = SUPLA_PROTO_VERSION;
  ve.server_version_min = SUPLA_PROTO_VERSION_MIN;

  return srpc_async__call(_srpc, SUPLA_SDC_CALL_VERSIONERROR, (char *)&ve,
                          sizeof(TSDC_SuplaVersionError), &remote_version);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_ping_server(void *_srpc) {
  TDCS_SuplaPingServer ps;

#if defined(ESP8266)
  unsigned int time = system_get_time();
  ps.now.tv_sec = time / 1000000;
  ps.now.tv_usec = time % 1000000;
#elif defined(__AVR__)
  ps.now.tv_sec[0] = 0;
  ps.now.tv_sec[1] = 0;
  ps.now.tv_usec[0] = 0;
  ps.now.tv_usec[1] = 0;
#else
  struct timeval now;
  gettimeofday(&now, NULL);
  ps.now.tv_sec = now.tv_sec;
  ps.now.tv_usec = now.tv_usec;
#endif

  return srpc_async_call(_srpc, SUPLA_DCS_CALL_PING_SERVER, (char *)&ps,
                         sizeof(TDCS_SuplaPingServer));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sdc_async_ping_server_result(void *_srpc) {
  (void)(_srpc);
#if !defined(ESP8266) && !defined(__AVR__) && !defined(ESP32) && \
    !defined(SUPLA_DEVICE)
  TSDC_SuplaPingServerResult ps;

  struct timeval now;
  gettimeofday(&now, NULL);
  ps.now.tv_sec = now.tv_sec;
  ps.now.tv_usec = now.tv_usec;

  return srpc_async_call(_srpc, SUPLA_SDC_CALL_PING_SERVER_RESULT, (char *)&ps,
                         sizeof(TSDC_SuplaPingServerResult));
#else
  return 0;
#endif
}

_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_set_activity_timeout(
    void *_srpc, TDCS_SuplaSetActivityTimeout *dcs_set_activity_timeout) {
  return srpc_async_call(_srpc, SUPLA_DCS_CALL_SET_ACTIVITY_TIMEOUT,
                         (char *)dcs_set_activity_timeout,
                         sizeof(TDCS_SuplaSetActivityTimeout));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_set_activity_timeout_result(
    void *_srpc,
    TSDC_SuplaSetActivityTimeoutResult *sdc_set_activity_timeout_result) {
  return srpc_async_call(_srpc, SUPLA_SDC_CALL_SET_ACTIVITY_TIMEOUT_RESULT,
                         (char *)sdc_set_activity_timeout_result,
                         sizeof(TSDC_SuplaSetActivityTimeoutResult));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_dcs_async_get_registration_enabled(void *_srpc) {
  return srpc_async_call(_srpc, SUPLA_DCS_CALL_GET_REGISTRATION_ENABLED, NULL,
                         0);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sdc_async_get_registration_enabled_result(
    void *_srpc, TSDC_RegistrationEnabled *reg_enabled) {
  return srpc_async_call(_srpc, SUPLA_SDC_CALL_GET_REGISTRATION_ENABLED_RESULT,
                         (char *)reg_enabled, sizeof(TSDC_RegistrationEnabled));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_dcs_async_get_user_localtime(void *_srpc) {
  return srpc_async_call(_srpc, SUPLA_DCS_CALL_GET_USER_LOCALTIME, NULL, 0);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sdc_async_get_user_localtime_result(
    void *_srpc, TSDC_UserLocalTimeResult *localtime) {
  if (localtime == NULL || localtime->timezoneSize > SUPLA_TIMEZONE_MAXSIZE) {
    return 0;
  }

  unsigned int size = sizeof(TSDC_UserLocalTimeResult) -
                      SUPLA_TIMEZONE_MAXSIZE + localtime->timezoneSize;

  return srpc_async_call(_srpc, SUPLA_DCS_CALL_GET_USER_LOCALTIME_RESULT,
                         (char *)localtime, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_csd_async_get_channel_state(
    void *_srpc, TCSD_ChannelStateRequest *request) {
  return srpc_async_call(_srpc, SUPLA_CSD_CALL_GET_CHANNEL_STATE,
                         (char *)request, sizeof(TCSD_ChannelStateRequest));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_csd_async_channel_state_result(void *_srpc, TDSC_ChannelState *state) {
  return srpc_async_call(_srpc, SUPLA_DSC_CALL_CHANNEL_STATE_RESULT,
                         (char *)state, sizeof(TDSC_ChannelState));
}

#ifndef SRPC_EXCLUDE_DEVICE
_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_get_firmware_update_url(
    void *_srpc, TDS_FirmwareUpdateParams *params) {
  return srpc_async_call(_srpc, SUPLA_DS_CALL_GET_FIRMWARE_UPDATE_URL,
                         (char *)params, sizeof(TDS_FirmwareUpdateParams));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice(
    void *_srpc, TDS_SuplaRegisterDevice *registerdevice) {
  _supla_int_t size =
      sizeof(TDS_SuplaRegisterDevice) -
      (sizeof(TDS_SuplaDeviceChannel) * SUPLA_CHANNELMAXCOUNT) +
      (sizeof(TDS_SuplaDeviceChannel) * registerdevice->channel_count);

  if (size > sizeof(TDS_SuplaRegisterDevice)) return 0;

  return srpc_async_call(_srpc, SUPLA_DS_CALL_REGISTER_DEVICE,
                         (char *)registerdevice, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_b(
    void *_srpc, TDS_SuplaRegisterDevice_B *registerdevice) {
  _supla_int_t size =
      sizeof(TDS_SuplaRegisterDevice_B) -
      (sizeof(TDS_SuplaDeviceChannel_B) * SUPLA_CHANNELMAXCOUNT) +
      (sizeof(TDS_SuplaDeviceChannel_B) * registerdevice->channel_count);

  if (size > sizeof(TDS_SuplaRegisterDevice_B)) return 0;

  return srpc_async_call(_srpc, SUPLA_DS_CALL_REGISTER_DEVICE_B,
                         (char *)registerdevice, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_c(
    void *_srpc, TDS_SuplaRegisterDevice_C *registerdevice) {
  _supla_int_t size =
      sizeof(TDS_SuplaRegisterDevice_C) -
      (sizeof(TDS_SuplaDeviceChannel_B) * SUPLA_CHANNELMAXCOUNT) +
      (sizeof(TDS_SuplaDeviceChannel_B) * registerdevice->channel_count);

  if (size > sizeof(TDS_SuplaRegisterDevice_C)) return 0;

  return srpc_async_call(_srpc, SUPLA_DS_CALL_REGISTER_DEVICE_C,
                         (char *)registerdevice, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_d(
    void *_srpc, TDS_SuplaRegisterDevice_D *registerdevice) {
  _supla_int_t size =
      sizeof(TDS_SuplaRegisterDevice_D) -
      (sizeof(TDS_SuplaDeviceChannel_B) * SUPLA_CHANNELMAXCOUNT) +
      (sizeof(TDS_SuplaDeviceChannel_B) * registerdevice->channel_count);

  if (size > sizeof(TDS_SuplaRegisterDevice_D)) return 0;

  return srpc_async_call(_srpc, SUPLA_DS_CALL_REGISTER_DEVICE_D,
                         (char *)registerdevice, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_e(
    void *_srpc, TDS_SuplaRegisterDevice_E *registerdevice) {
  _supla_int_t size =
      sizeof(TDS_SuplaRegisterDevice_E) -
      (sizeof(TDS_SuplaDeviceChannel_C) * SUPLA_CHANNELMAXCOUNT) +
      (sizeof(TDS_SuplaDeviceChannel_C) * registerdevice->channel_count);

  if (size > sizeof(TDS_SuplaRegisterDevice_E)) return 0;

  return srpc_async_call(_srpc, SUPLA_DS_CALL_REGISTER_DEVICE_E,
                         (char *)registerdevice, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_f(
    void *_srpc, TDS_SuplaRegisterDevice_F *registerdevice) {
  _supla_int_t size =
      sizeof(TDS_SuplaRegisterDevice_F) -
      (sizeof(TDS_SuplaDeviceChannel_D) * SUPLA_CHANNELMAXCOUNT) +
      (sizeof(TDS_SuplaDeviceChannel_D) * registerdevice->channel_count);

  if (size > sizeof(TDS_SuplaRegisterDevice_F)) return 0;

  return srpc_async_call(_srpc, SUPLA_DS_CALL_REGISTER_DEVICE_F,
                         (char *)registerdevice, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_registerdevice_result(
    void *_srpc, TSD_SuplaRegisterDeviceResult *registerdevice_result) {
  return srpc_async_call(_srpc, SUPLA_SD_CALL_REGISTER_DEVICE_RESULT,
                         (char *)registerdevice_result,
                         sizeof(TSD_SuplaRegisterDeviceResult));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_sd_async_set_channel_value(void *_srpc, TSD_SuplaChannelNewValue *value) {
  return srpc_async_call(_srpc, SUPLA_SD_CALL_CHANNEL_SET_VALUE, (char *)value,
                         sizeof(TSD_SuplaChannelNewValue));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_set_channelgroup_value(
    void *_srpc, TSD_SuplaChannelGroupNewValue *value) {
  return srpc_async_call(_srpc, SUPLA_SD_CALL_CHANNELGROUP_SET_VALUE,
                         (char *)value, sizeof(TSD_SuplaChannelGroupNewValue));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_ds_async_set_channel_result(void *_srpc, unsigned char ChannelNumber,
                                 _supla_int_t SenderID, char Success) {
  TDS_SuplaChannelNewValueResult result;
  result.ChannelNumber = ChannelNumber;
  result.SenderID = SenderID;
  result.Success = Success;

  return srpc_async_call(_srpc, SUPLA_DS_CALL_CHANNEL_SET_VALUE_RESULT,
                         (char *)&result,
                         sizeof(TDS_SuplaChannelNewValueResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_get_firmware_update_url_result(
    void *_srpc, TSD_FirmwareUpdate_UrlResult *result) {
  return srpc_async_call(
      _srpc, SUPLA_SD_CALL_GET_FIRMWARE_UPDATE_URL_RESULT, (char *)result,
      result->exists == 1 ? sizeof(TSD_FirmwareUpdate_UrlResult)
                          : sizeof(char));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_channel_value_changed(
    void *_srpc, unsigned char channel_number, char *value) {
  TDS_SuplaDeviceChannelValue ncsc;
  ncsc.ChannelNumber = channel_number;
  memcpy(ncsc.value, value, SUPLA_CHANNELVALUE_SIZE);

  return srpc_async_call(_srpc, SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED,
                         (char *)&ncsc, sizeof(TDS_SuplaDeviceChannelValue));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_ds_async_channel_value_changed_b(void *_srpc, unsigned char channel_number,
                                      char *value, unsigned char offline) {
  TDS_SuplaDeviceChannelValue_B ncsc;
  ncsc.ChannelNumber = channel_number;
  ncsc.Offline = !!offline;
  memcpy(ncsc.value, value, SUPLA_CHANNELVALUE_SIZE);

  return srpc_async_call(_srpc, SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_B,
                         (char *)&ncsc, sizeof(TDS_SuplaDeviceChannelValue_B));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_channel_value_changed_c(
    void *_srpc, unsigned char channel_number, char *value,
    unsigned char offline, unsigned _supla_int_t validity_time_sec) {
  TDS_SuplaDeviceChannelValue_C ncsc;
  ncsc.ChannelNumber = channel_number;
  ncsc.Offline = !!offline;
  ncsc.ValidityTimeSec = validity_time_sec;
  memcpy(ncsc.value, value, SUPLA_CHANNELVALUE_SIZE);

  return srpc_async_call(_srpc, SUPLA_DS_CALL_DEVICE_CHANNEL_VALUE_CHANGED_C,
                         (char *)&ncsc, sizeof(TDS_SuplaDeviceChannelValue_C));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_channel_extendedvalue_changed(
    void *_srpc, unsigned char channel_number,
    TSuplaChannelExtendedValue *value) {
  if (value == NULL || value->size > SUPLA_CHANNELEXTENDEDVALUE_SIZE ||
      value->size == 0) {
    return 0;
  }

  TDS_SuplaDeviceChannelExtendedValue ncsc;
  ncsc.ChannelNumber = channel_number;
  memcpy(&ncsc.value, value, sizeof(TSuplaChannelExtendedValue));

  return srpc_async_call(
      _srpc, SUPLA_DS_CALL_DEVICE_CHANNEL_EXTENDEDVALUE_CHANGED, (char *)&ncsc,
      sizeof(TDS_SuplaDeviceChannelExtendedValue) -
          (SUPLA_CHANNELEXTENDEDVALUE_SIZE - ncsc.value.size));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_device_calcfg_request(
    void *_srpc, TSD_DeviceCalCfgRequest *request) {
  if (request == NULL || request->DataSize > SUPLA_CALCFG_DATA_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SD_CALL_DEVICE_CALCFG_REQUEST,
                         (char *)request,
                         sizeof(TSD_DeviceCalCfgRequest) -
                             SUPLA_CALCFG_DATA_MAXSIZE + request->DataSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_device_calcfg_result(
    void *_srpc, TDS_DeviceCalCfgResult *result) {
  if (result == NULL || result->DataSize > SUPLA_CALCFG_DATA_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_DS_CALL_DEVICE_CALCFG_RESULT,
                         (char *)result,
                         sizeof(TDS_DeviceCalCfgResult) -
                             SUPLA_CALCFG_DATA_MAXSIZE + result->DataSize);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_ds_async_get_channel_functions(void *_srpc) {
  return srpc_async_call(_srpc, SUPLA_DS_CALL_GET_CHANNEL_FUNCTIONS, NULL, 0);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_get_channel_functions_result(
    void *_srpc, TSD_ChannelFunctions *result) {
  if (result == NULL || result->ChannelCount > SUPLA_CHANNELMAXCOUNT) {
    return 0;
  }

  _supla_int_t size = sizeof(TSD_ChannelFunctions) -
                      sizeof(_supla_int_t) * SUPLA_CHANNELMAXCOUNT +
                      sizeof(_supla_int_t) * result->ChannelCount;

  if (size > sizeof(TSD_ChannelFunctions)) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SD_CALL_GET_CHANNEL_FUNCTIONS_RESULT,
                         (char *)result, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_get_channel_config_request(
    void *_srpc, TDS_GetChannelConfigRequest *request) {
  if (request == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_DS_CALL_GET_CHANNEL_CONFIG,
                         (char *)request, sizeof(TDS_GetChannelConfigRequest));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_get_channel_config_result(
    void *_srpc, TSD_ChannelConfig *config) {
  if (config == NULL || config->ConfigSize > SUPLA_CHANNEL_CONFIG_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SD_CALL_GET_CHANNEL_CONFIG_RESULT,
                         (char *)config,
                         sizeof(TSD_ChannelConfig) -
                             SUPLA_CHANNEL_CONFIG_MAXSIZE + config->ConfigSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_set_channel_config_request(
    void *_srpc, TSDS_SetChannelConfig *config) {
  if (config == NULL || config->ConfigSize > SUPLA_CHANNEL_CONFIG_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SD_CALL_SET_CHANNEL_CONFIG,
                         (char *)config,
                         sizeof(TSDS_SetChannelConfig) -
                             SUPLA_CHANNEL_CONFIG_MAXSIZE + config->ConfigSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_set_channel_config_result(
    void *_srpc, TSDS_SetChannelConfigResult *result) {
  if (result == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_DS_CALL_SET_CHANNEL_CONFIG_RESULT,
                         (char *)result, sizeof(TSDS_SetChannelConfigResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_set_channel_config_request(
    void *_srpc, TSDS_SetChannelConfig *config) {
  if (config == NULL || config->ConfigSize > SUPLA_CHANNEL_CONFIG_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_DS_CALL_SET_CHANNEL_CONFIG,
                         (char *)config,
                         sizeof(TSDS_SetChannelConfig) -
                             SUPLA_CHANNEL_CONFIG_MAXSIZE + config->ConfigSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_set_channel_config_result(
    void *_srpc, TSDS_SetChannelConfigResult *result) {
  if (result == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SD_CALL_SET_CHANNEL_CONFIG_RESULT,
                         (char *)result, sizeof(TSDS_SetChannelConfigResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_channel_config_finished(
    void *_srpc, TSD_ChannelConfigFinished *fin) {
  if (fin == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SD_CALL_CHANNEL_CONFIG_FINISHED,
                         (char *)fin, sizeof(TSD_ChannelConfigFinished));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_set_device_config_request(
    void *_srpc, TSDS_SetDeviceConfig *config) {
  if (config == NULL || config->ConfigSize > SUPLA_DEVICE_CONFIG_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SD_CALL_SET_DEVICE_CONFIG, (char *)config,
                         sizeof(TSDS_SetDeviceConfig) -
                             SUPLA_DEVICE_CONFIG_MAXSIZE + config->ConfigSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_set_device_config_result(
    void *_srpc, TSDS_SetDeviceConfigResult *result) {
  if (result == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_DS_CALL_SET_DEVICE_CONFIG_RESULT,
                         (char *)result, sizeof(TSDS_SetDeviceConfigResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_set_device_config_request(
    void *_srpc, TSDS_SetDeviceConfig *config) {
  if (config == NULL || config->ConfigSize > SUPLA_DEVICE_CONFIG_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_DS_CALL_SET_DEVICE_CONFIG, (char *)config,
                         sizeof(TSDS_SetDeviceConfig) -
                             SUPLA_DEVICE_CONFIG_MAXSIZE + config->ConfigSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sd_async_set_device_config_result(
    void *_srpc, TSDS_SetDeviceConfigResult *result) {
  if (result == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SD_CALL_SET_DEVICE_CONFIG_RESULT,
                         (char *)result, sizeof(TSDS_SetDeviceConfigResult));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_ds_async_action_trigger(void *_srpc, TDS_ActionTrigger *action_trigger) {
  if (action_trigger == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_DS_CALL_ACTIONTRIGGER,
                         (char *)action_trigger, sizeof(TDS_ActionTrigger));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_register_push_notification(
    void *_srpc, TDS_RegisterPushNotification *reg) {
  if (!reg) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_DS_CALL_REGISTER_PUSH_NOTIFICATION,
                         (char *)reg, sizeof(TDS_RegisterPushNotification));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_ds_async_send_push_notification(void *_srpc, TDS_PushNotification *push) {
  if (!push || push->BodySize > SUPLA_PN_BODY_MAXSIZE ||
      push->TitleSize > SUPLA_PN_TITLE_MAXSIZE) {
    return 0;
  }

  unsigned _supla_int_t size = sizeof(TDS_PushNotification) -
                               SUPLA_PN_BODY_MAXSIZE - SUPLA_PN_TITLE_MAXSIZE +
                               push->BodySize + push->TitleSize;

  return srpc_async_call(_srpc, SUPLA_DS_CALL_SEND_PUSH_NOTIFICATION,
                         (char *)push, size);
}

#endif /*SRPC_EXCLUDE_DEVICE*/

#ifndef SRPC_EXCLUDE_CLIENT
_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_registerclient(
    void *_srpc, TCS_SuplaRegisterClient *registerclient) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_REGISTER_CLIENT,
                         (char *)registerclient,
                         sizeof(TCS_SuplaRegisterClient));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_registerclient_b(
    void *_srpc, TCS_SuplaRegisterClient_B *registerclient) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_REGISTER_CLIENT_B,
                         (char *)registerclient,
                         sizeof(TCS_SuplaRegisterClient_B));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_registerclient_c(
    void *_srpc, TCS_SuplaRegisterClient_C *registerclient) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_REGISTER_CLIENT_C,
                         (char *)registerclient,
                         sizeof(TCS_SuplaRegisterClient_C));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_registerclient_d(
    void *_srpc, TCS_SuplaRegisterClient_D *registerclient) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_REGISTER_CLIENT_D,
                         (char *)registerclient,
                         sizeof(TCS_SuplaRegisterClient_D));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_registerclient_result(
    void *_srpc, TSC_SuplaRegisterClientResult *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_REGISTER_CLIENT_RESULT,
                         (char *)result, sizeof(TSC_SuplaRegisterClientResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_registerclient_result_b(
    void *_srpc, TSC_SuplaRegisterClientResult_B *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_B,
                         (char *)result,
                         sizeof(TSC_SuplaRegisterClientResult_B));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_registerclient_result_c(
    void *_srpc, TSC_SuplaRegisterClientResult_C *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_C,
                         (char *)result,
                         sizeof(TSC_SuplaRegisterClientResult_C));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_registerclient_result_d(
    void *_srpc, TSC_SuplaRegisterClientResult_D *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_REGISTER_CLIENT_RESULT_D,
                         (char *)result,
                         sizeof(TSC_SuplaRegisterClientResult_D));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_sc_async_location_update(void *_srpc, TSC_SuplaLocation *location) {
  _supla_int_t size = sizeof(TSC_SuplaLocation) -
                      SUPLA_LOCATION_CAPTION_MAXSIZE + location->CaptionSize;

  if (size > sizeof(TSC_SuplaLocation)) return 0;

  return srpc_async_call(_srpc, SUPLA_SC_CALL_LOCATION_UPDATE, (char *)location,
                         size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_set_pack(
    void *_srpc, void *pack, _supla_int_t count,
    _func_srpc_pack_get_caption_size get_caption_size,
    _func_srpc_pack_get_item_ptr get_item_ptr,
    _func_srpc_pack_set_pack_count set_pack_count,
    unsigned _supla_int_t pack_sizeof, unsigned _supla_int_t pack_max_count,
    unsigned _supla_int_t caption_max_size, unsigned _supla_int_t item_sizeof,
    unsigned _supla_int_t call_id) {
  _supla_int_t result = 0;
  _supla_int_t a;
  _supla_int_t n = 0;
  _supla_int_t size = 0;
  _supla_int_t offset = 0;

  if (count < 1 || count > pack_max_count) return 0;

  size = pack_sizeof - (item_sizeof * pack_max_count);
  offset = size;

  char *buffer = malloc(size);

  if (buffer == NULL) return 0;

  memcpy(buffer, pack, size);

  for (a = 0; a < count; a++) {
    if (get_caption_size(pack, a) <= caption_max_size) {
      size += item_sizeof - caption_max_size + get_caption_size(pack, a);

      char *new_buffer = (char *)realloc(buffer, size);

      if (new_buffer == NULL) {
        free(buffer);
        return 0;
      }

      buffer = new_buffer;
      memcpy(&buffer[offset], get_item_ptr(pack, a), size - offset);
      offset += size - offset;
      n++;
    }
  }

  set_pack_count(buffer, n, 0);

  result = srpc_async_call(_srpc, call_id, buffer, size);

  free(buffer);
  return result;
}

unsigned _supla_int_t srpc_locationpack_get_caption_size(void *pack,
                                                         _supla_int_t idx) {
  return ((TSC_SuplaLocationPack *)pack)->items[idx].CaptionSize;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_locationpack_update(
    void *_srpc, TSC_SuplaLocationPack *location_pack) {
  return srpc_set_pack(
      _srpc, location_pack, location_pack->count,
      &srpc_locationpack_get_caption_size, &srpc_locationpack_get_item_ptr,
      &srpc_locationpack_set_pack_count, sizeof(TSC_SuplaLocationPack),
      SUPLA_LOCATIONPACK_MAXCOUNT, SUPLA_LOCATION_CAPTION_MAXSIZE,
      sizeof(TSC_SuplaLocation), SUPLA_SC_CALL_LOCATIONPACK_UPDATE);
}

unsigned _supla_int_t srpc_channelpack_get_caption_size(void *pack,
                                                        _supla_int_t idx) {
  return ((TSC_SuplaChannelPack *)pack)->items[idx].CaptionSize;
}

unsigned _supla_int_t srpc_channelpack_get_caption_size_b(void *pack,
                                                          _supla_int_t idx) {
  return ((TSC_SuplaChannelPack_B *)pack)->items[idx].CaptionSize;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelpack_update(
    void *_srpc, TSC_SuplaChannelPack *channel_pack) {
  return srpc_set_pack(
      _srpc, channel_pack, channel_pack->count,
      &srpc_channelpack_get_caption_size, &srpc_channelpack_get_item_ptr,
      &srpc_channelpack_set_pack_count, sizeof(TSC_SuplaChannelPack),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      sizeof(TSC_SuplaChannel), SUPLA_SC_CALL_CHANNELPACK_UPDATE);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelpack_update_b(
    void *_srpc, TSC_SuplaChannelPack_B *channel_pack) {
  return srpc_set_pack(
      _srpc, channel_pack, channel_pack->count,
      &srpc_channelpack_get_caption_size_b, &srpc_channelpack_get_item_ptr_b,
      &srpc_channelpack_set_pack_count_b, sizeof(TSC_SuplaChannelPack_B),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      sizeof(TSC_SuplaChannel_B), SUPLA_SC_CALL_CHANNELPACK_UPDATE_B);
}

unsigned _supla_int_t srpc_channelpack_get_caption_size_c(void *pack,
                                                          _supla_int_t idx) {
  return ((TSC_SuplaChannelPack_C *)pack)->items[idx].CaptionSize;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelpack_update_c(
    void *_srpc, TSC_SuplaChannelPack_C *channel_pack) {
  return srpc_set_pack(
      _srpc, channel_pack, channel_pack->count,
      &srpc_channelpack_get_caption_size_c, &srpc_channelpack_get_item_ptr_c,
      &srpc_channelpack_set_pack_count_c, sizeof(TSC_SuplaChannelPack_C),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      sizeof(TSC_SuplaChannel_C), SUPLA_SC_CALL_CHANNELPACK_UPDATE_C);
}

unsigned _supla_int_t srpc_channelpack_get_caption_size_d(void *pack,
                                                          _supla_int_t idx) {
  return ((TSC_SuplaChannelPack_D *)pack)->items[idx].CaptionSize;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelpack_update_d(
    void *_srpc, TSC_SuplaChannelPack_D *channel_pack) {
  return srpc_set_pack(
      _srpc, channel_pack, channel_pack->count,
      &srpc_channelpack_get_caption_size_d, &srpc_channelpack_get_item_ptr_d,
      &srpc_channelpack_set_pack_count_d, sizeof(TSC_SuplaChannelPack_D),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      sizeof(TSC_SuplaChannel_D), SUPLA_SC_CALL_CHANNELPACK_UPDATE_D);
}

unsigned _supla_int_t srpc_channelpack_get_caption_size_e(void *pack,
                                                          _supla_int_t idx) {
  return ((TSC_SuplaChannelPack_E *)pack)->items[idx].CaptionSize;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelpack_update_e(
    void *_srpc, TSC_SuplaChannelPack_E *channel_pack) {
  return srpc_set_pack(
      _srpc, channel_pack, channel_pack->count,
      &srpc_channelpack_get_caption_size_e, &srpc_channelpack_get_item_ptr_e,
      &srpc_channelpack_set_pack_count_e, sizeof(TSC_SuplaChannelPack_E),
      SUPLA_CHANNELPACK_MAXCOUNT, SUPLA_CHANNEL_CAPTION_MAXSIZE,
      sizeof(TSC_SuplaChannel_E), SUPLA_SC_CALL_CHANNELPACK_UPDATE_E);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channel_value_update(
    void *_srpc, TSC_SuplaChannelValue *channel_value) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE,
                         (char *)channel_value, sizeof(TSC_SuplaChannelValue));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channel_value_update_b(
    void *_srpc, TSC_SuplaChannelValue_B *channel_value) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNEL_VALUE_UPDATE_B,
                         (char *)channel_value,
                         sizeof(TSC_SuplaChannelValue_B));
}

unsigned _supla_int_t
srpc_channelgroup_pack_get_caption_size(void *pack, _supla_int_t idx) {
  return ((TSC_SuplaChannelGroupPack *)pack)->items[idx].CaptionSize;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelgroup_pack_update(
    void *_srpc, TSC_SuplaChannelGroupPack *channelgroup_pack) {
  return srpc_set_pack(
      _srpc, channelgroup_pack, channelgroup_pack->count,
      &srpc_channelgroup_pack_get_caption_size,
      &srpc_channelgroup_pack_get_item_ptr,
      &srpc_channelgroup_pack_set_pack_count, sizeof(TSC_SuplaChannelGroupPack),
      SUPLA_CHANNELGROUP_PACK_MAXCOUNT, SUPLA_CHANNELGROUP_CAPTION_MAXSIZE,
      sizeof(TSC_SuplaChannelGroup), SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE);
}

unsigned _supla_int_t
srpc_channelgroup_pack_b_get_caption_size(void *pack, _supla_int_t idx) {
  return ((TSC_SuplaChannelGroupPack_B *)pack)->items[idx].CaptionSize;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelgroup_pack_update_b(
    void *_srpc, TSC_SuplaChannelGroupPack_B *channelgroup_pack) {
  return srpc_set_pack(
      _srpc, channelgroup_pack, channelgroup_pack->count,
      &srpc_channelgroup_pack_b_get_caption_size,
      &srpc_channelgroup_pack_b_get_item_ptr,
      &srpc_channelgroup_pack_b_set_pack_count,
      sizeof(TSC_SuplaChannelGroupPack_B), SUPLA_CHANNELGROUP_PACK_MAXCOUNT,
      SUPLA_CHANNELGROUP_CAPTION_MAXSIZE, sizeof(TSC_SuplaChannelGroup_B),
      SUPLA_SC_CALL_CHANNELGROUP_PACK_UPDATE_B);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelgroup_relation_pack_update(
    void *_srpc,
    TSC_SuplaChannelGroupRelationPack *channelgroup_relation_pack) {
  if (channelgroup_relation_pack->count < 1 ||
      channelgroup_relation_pack->count >
          SUPLA_CHANNELGROUP_RELATION_PACK_MAXCOUNT) {
    return 0;
  }

  unsigned _supla_int_t size = sizeof(TSC_SuplaChannelGroupRelationPack) -
                               sizeof(channelgroup_relation_pack->items) +
                               (sizeof(TSC_SuplaChannelGroupRelation) *
                                channelgroup_relation_pack->count);

  return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNELGROUP_RELATION_PACK_UPDATE,
                         (char *)channelgroup_relation_pack, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channel_relation_pack_update(
    void *_srpc, TSC_SuplaChannelRelationPack *relation_pack) {
  if (relation_pack->count < 1 ||
      relation_pack->count > SUPLA_CHANNEL_RELATION_PACK_MAXCOUNT) {
    return 0;
  }

  unsigned _supla_int_t size =
      sizeof(TSC_SuplaChannelRelationPack) - sizeof(relation_pack->items) +
      (sizeof(TSC_SuplaChannelRelation) * relation_pack->count);

  return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNEL_RELATION_PACK_UPDATE,
                         (char *)relation_pack, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelvalue_pack_update(
    void *_srpc, TSC_SuplaChannelValuePack *channelvalue_pack) {
  if (channelvalue_pack->count < 1 ||
      channelvalue_pack->count > SUPLA_CHANNELVALUE_PACK_MAXCOUNT) {
    return 0;
  }

  unsigned _supla_int_t size =
      sizeof(TSC_SuplaChannelValuePack) - sizeof(channelvalue_pack->items) +
      (sizeof(TSC_SuplaChannelValue) * channelvalue_pack->count);

  return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE,
                         (char *)channelvalue_pack, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelvalue_pack_update_b(
    void *_srpc, TSC_SuplaChannelValuePack_B *channelvalue_pack) {
  if (channelvalue_pack->count < 1 ||
      channelvalue_pack->count > SUPLA_CHANNELVALUE_PACK_MAXCOUNT) {
    return 0;
  }

  unsigned _supla_int_t size =
      sizeof(TSC_SuplaChannelValuePack_B) - sizeof(channelvalue_pack->items) +
      (sizeof(TSC_SuplaChannelValue_B) * channelvalue_pack->count);

  return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNELVALUE_PACK_UPDATE_B,
                         (char *)channelvalue_pack, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channelextendedvalue_pack_update(
    void *_srpc, TSC_SuplaChannelExtendedValuePack *extendedvalue_pack) {
  if (extendedvalue_pack == NULL || extendedvalue_pack->count < 1 ||
      extendedvalue_pack->count > SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXCOUNT ||
      extendedvalue_pack->pack_size < 1 ||
      extendedvalue_pack->pack_size >
          SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNELEXTENDEDVALUE_PACK_UPDATE,
                         (char *)extendedvalue_pack,
                         sizeof(TSC_SuplaChannelExtendedValuePack) -
                             SUPLA_CHANNELEXTENDEDVALUE_PACK_MAXDATASIZE +
                             extendedvalue_pack->pack_size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_get_next(void *_srpc) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_GET_NEXT, NULL, 0);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_event(void *_srpc,
                                                   TSC_SuplaEvent *event) {
  _supla_int_t size = sizeof(TSC_SuplaEvent) - SUPLA_SENDER_NAME_MAXSIZE +
                      event->SenderNameSize;

  if (size > sizeof(TSC_SuplaEvent)) return 0;

  return srpc_async_call(_srpc, SUPLA_SC_CALL_EVENT, (char *)event, size);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_channel_value(void *_srpc, TCS_SuplaChannelNewValue *value) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_CHANNEL_SET_VALUE, (char *)value,
                         sizeof(TCS_SuplaChannelNewValue));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_set_channel_value_b(
    void *_srpc, TCS_SuplaChannelNewValue_B *value) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_CHANNEL_SET_VALUE_B,
                         (char *)value, sizeof(TCS_SuplaChannelNewValue_B));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_value(void *_srpc, TCS_SuplaNewValue *value) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_SET_VALUE, (char *)value,
                         sizeof(TCS_SuplaNewValue));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_oauth_token_request(void *_srpc) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_OAUTH_TOKEN_REQUEST, NULL, 0);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_oauth_token_request_result(
    void *_srpc, TSC_OAuthTokenRequestResult *result) {
  if (result == NULL || result->Token.TokenSize > SUPLA_OAUTH_TOKEN_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(
      _srpc, SUPLA_SC_CALL_OAUTH_TOKEN_REQUEST_RESULT, (char *)result,
      sizeof(TSC_OAuthTokenRequestResult) -
          (SUPLA_OAUTH_TOKEN_MAXSIZE - result->Token.TokenSize));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_superuser_authorization_request(
    void *_srpc, TCS_SuperUserAuthorizationRequest *request) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_SUPERUSER_AUTHORIZATION_REQUEST,
                         (char *)request,
                         sizeof(TCS_SuperUserAuthorizationRequest));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_get_superuser_authorization_result(void *_srpc) {
  return srpc_async_call(
      _srpc, SUPLA_CS_CALL_GET_SUPERUSER_AUTHORIZATION_RESULT, NULL, 0);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_superuser_authorization_result(
    void *_srpc, TSC_SuperUserAuthorizationResult *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_SUPERUSER_AUTHORIZATION_RESULT,
                         (char *)result,
                         sizeof(TSC_SuperUserAuthorizationResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_device_calcfg_request(
    void *_srpc, TCS_DeviceCalCfgRequest *request) {
  if (request == NULL || request->DataSize > SUPLA_CALCFG_DATA_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST,
                         (char *)request,
                         sizeof(TCS_DeviceCalCfgRequest) -
                             SUPLA_CALCFG_DATA_MAXSIZE + request->DataSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_device_calcfg_request_b(
    void *_srpc, TCS_DeviceCalCfgRequest_B *request) {
  if (request == NULL || request->DataSize > SUPLA_CALCFG_DATA_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_CS_CALL_DEVICE_CALCFG_REQUEST_B,
                         (char *)request,
                         sizeof(TCS_DeviceCalCfgRequest_B) -
                             SUPLA_CALCFG_DATA_MAXSIZE + request->DataSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_device_calcfg_result(
    void *_srpc, TSC_DeviceCalCfgResult *result) {
  if (result == NULL || result->DataSize > SUPLA_CALCFG_DATA_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SC_CALL_DEVICE_CALCFG_RESULT,
                         (char *)result,
                         sizeof(TSC_DeviceCalCfgResult) -
                             SUPLA_CALCFG_DATA_MAXSIZE + result->DataSize);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_get_channel_basic_cfg(void *_srpc, _supla_int_t ChannelID) {
  TCS_ChannelBasicCfgRequest request;
  memset(&request, 0, sizeof(TCS_ChannelBasicCfgRequest));
  request.ChannelID = ChannelID;

  return srpc_async_call(_srpc, SUPLA_CS_CALL_GET_CHANNEL_BASIC_CFG,
                         (char *)&request, sizeof(TCS_ChannelBasicCfgRequest));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channel_basic_cfg_result(
    void *_srpc, TSC_ChannelBasicCfg *basic_cfg) {
  _supla_int_t size = sizeof(TSC_ChannelBasicCfg) -
                      SUPLA_CHANNEL_CAPTION_MAXSIZE + basic_cfg->CaptionSize;

  if (size > sizeof(TSC_ChannelBasicCfg)) return 0;

  return srpc_async_call(_srpc, SUPLA_SC_CALL_CHANNEL_BASIC_CFG_RESULT,
                         (char *)basic_cfg, size);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_channel_function(void *_srpc, TCS_SetChannelFunction *func) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_SET_CHANNEL_FUNCTION,
                         (char *)func, sizeof(TCS_SetChannelFunction));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_set_channel_function_result(
    void *_srpc, TSC_SetChannelFunctionResult *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_SET_CHANNEL_FUNCTION_RESULT,
                         (char *)result, sizeof(TSC_SetChannelFunctionResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_set_caption(
    void *_srpc, TCS_SetCaption *caption, unsigned _supla_int_t call_id) {
  _supla_int_t size =
      sizeof(TCS_SetCaption) - SUPLA_CAPTION_MAXSIZE + caption->CaptionSize;

  if (size > sizeof(TCS_SetCaption)) return 0;

  return srpc_async_call(_srpc, call_id, (char *)caption, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_set_caption_result(
    void *_srpc, TSC_SetCaptionResult *result, unsigned _supla_int_t call_id) {
  _supla_int_t size = sizeof(TSC_SetCaptionResult) - SUPLA_CAPTION_MAXSIZE +
                      result->CaptionSize;

  if (size > sizeof(TSC_SetCaptionResult)) return 0;

  return srpc_async_call(_srpc, call_id, (char *)result, size);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_channel_caption(void *_srpc, TCS_SetCaption *caption) {
  return srpc_cs_async_set_caption(_srpc, caption,
                                   SUPLA_CS_CALL_SET_CHANNEL_CAPTION);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_set_channel_caption_result(
    void *_srpc, TSC_SetCaptionResult *result) {
  return srpc_sc_async_set_caption_result(
      _srpc, result, SUPLA_SC_CALL_SET_CHANNEL_CAPTION_RESULT);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_channel_group_caption(void *_srpc, TCS_SetCaption *caption) {
  return srpc_cs_async_set_caption(_srpc, caption,
                                   SUPLA_CS_CALL_SET_CHANNEL_GROUP_CAPTION);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_set_channel_group_caption_result(
    void *_srpc, TSC_SetCaptionResult *result) {
  return srpc_sc_async_set_caption_result(
      _srpc, result, SUPLA_SC_CALL_SET_CHANNEL_GROUP_CAPTION_RESULT);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_location_caption(void *_srpc, TCS_SetCaption *caption) {
  return srpc_cs_async_set_caption(_srpc, caption,
                                   SUPLA_CS_CALL_SET_LOCATION_CAPTION);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_set_location_caption_result(
    void *_srpc, TSC_SetCaptionResult *result) {
  return srpc_sc_async_set_caption_result(
      _srpc, result, SUPLA_SC_CALL_SET_LOCATION_CAPTION_RESULT);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_set_scene_caption(void *_srpc, TCS_SetCaption *caption) {
  return srpc_cs_async_set_caption(_srpc, caption,
                                   SUPLA_CS_CALL_SET_SCENE_CAPTION);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_set_scene_caption_result(
    void *_srpc, TSC_SetCaptionResult *result) {
  return srpc_sc_async_set_caption_result(
      _srpc, result, SUPLA_SC_CALL_SET_SCENE_CAPTION_RESULT);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_clients_reconnect_request(void *_srpc) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_CLIENTS_RECONNECT_REQUEST, NULL,
                         0);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_clients_reconnect_request_result(
    void *_srpc, TSC_ClientsReconnectRequestResult *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_CLIENTS_RECONNECT_REQUEST_RESULT,
                         (char *)result,
                         sizeof(TSC_ClientsReconnectRequestResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_set_registration_enabled(
    void *_srpc, TCS_SetRegistrationEnabled *reg_enabled) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_SET_REGISTRATION_ENABLED,
                         (char *)reg_enabled,
                         sizeof(TCS_SetRegistrationEnabled));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_set_registration_enabled_result(
    void *_srpc, TSC_SetRegistrationEnabledResult *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_SET_REGISTRATION_ENABLED_RESULT,
                         (char *)result,
                         sizeof(TSC_SetRegistrationEnabledResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_device_reconnect_request(
    void *_srpc, TCS_DeviceReconnectRequest *request) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_DEVICE_RECONNECT_REQUEST,
                         (char *)request, sizeof(TCS_DeviceReconnectRequest));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_device_reconnect_request_result(
    void *_srpc, TSC_DeviceReconnectRequestResult *result) {
  return srpc_async_call(_srpc, SUPLA_SC_CALL_DEVICE_RECONNECT_REQUEST_RESULT,
                         (char *)result,
                         sizeof(TSC_DeviceReconnectRequestResult));
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_timer_arm(void *_srpc, TCS_TimerArmRequest *request) {
  return srpc_async_call(_srpc, SUPLA_CS_CALL_TIMER_ARM, (char *)request,
                         sizeof(TCS_TimerArmRequest));
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_scenepack_get_caption_size(void *pack, _supla_int_t idx) {
  return ((TSC_SuplaScenePack *)pack)->items[idx].CaptionSize;
}

void *SRPC_ICACHE_FLASH srpc_scenepack_get_item_ptr(void *pack,
                                                    _supla_int_t idx) {
  return &((TSC_SuplaScenePack *)pack)->items[idx];  // NOLINT
}

void SRPC_ICACHE_FLASH srpc_scenepack_set_pack_count(void *pack,
                                                     _supla_int_t count,
                                                     unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaScenePack *)pack)->count = count;
  } else {
    ((TSC_SuplaScenePack *)pack)->count += count;
  }
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_sc_async_scene_pack_update(void *_srpc, TSC_SuplaScenePack *scene_pack) {
  return srpc_set_pack(
      _srpc, scene_pack, scene_pack->count, &srpc_scenepack_get_caption_size,
      &srpc_scenepack_get_item_ptr, &srpc_scenepack_set_pack_count,
      sizeof(TSC_SuplaScenePack), SUPLA_SCENE_PACK_MAXCOUNT,
      SUPLA_SCENE_CAPTION_MAXSIZE, sizeof(TSC_SuplaScene),
      SUPLA_SC_CALL_SCENE_PACK_UPDATE);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_scenepack_get_pack_count(void *pack) {
  return ((TSC_SuplaScenePack *)pack)->count;
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_scenepack_get_item_caption_size(void *item) {
  return ((TSC_SuplaScene *)item)->CaptionSize;
}

void SRPC_ICACHE_FLASH srpc_get_scene_pack(Tsrpc *srpc, TsrpcReceivedData *rd) {
  srpc_getpack(srpc, rd, sizeof(TSC_SuplaScenePack), sizeof(TSC_SuplaScene),
               SUPLA_SCENE_PACK_MAXCOUNT, SUPLA_SCENE_CAPTION_MAXSIZE,
               &srpc_scenepack_get_pack_count, &srpc_scenepack_set_pack_count,
               &srpc_scenepack_get_item_ptr,
               &srpc_scenepack_get_item_caption_size);
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_scenestatepack_get_initiatorname_size(void *pack, _supla_int_t idx) {
  return ((TSC_SuplaSceneStatePack *)pack)->items[idx].InitiatorNameSize;
}

void *SRPC_ICACHE_FLASH srpc_scenestatepack_get_item_ptr(void *pack,
                                                         _supla_int_t idx) {
  return &((TSC_SuplaSceneStatePack *)pack)->items[idx];  // NOLINT
}

void SRPC_ICACHE_FLASH srpc_scenestatepack_set_pack_count(
    void *pack, _supla_int_t count, unsigned char increment) {
  if (increment == 0) {
    ((TSC_SuplaSceneStatePack *)pack)->count = count;
  } else {
    ((TSC_SuplaSceneStatePack *)pack)->count += count;
  }
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_scene_state_pack_update(
    void *_srpc, TSC_SuplaSceneStatePack *scene_state_pack) {
  return srpc_set_pack(
      _srpc, scene_state_pack, scene_state_pack->count,
      &srpc_scenestatepack_get_initiatorname_size,
      &srpc_scenestatepack_get_item_ptr, &srpc_scenestatepack_set_pack_count,
      sizeof(TSC_SuplaSceneStatePack), SUPLA_SCENE_STATE_PACK_MAXCOUNT,
      SUPLA_INITIATOR_NAME_MAXSIZE, sizeof(TSC_SuplaSceneState),
      SUPLA_SC_CALL_SCENE_STATE_PACK_UPDATE);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_scenestatepack_get_pack_count(void *pack) {
  return ((TSC_SuplaSceneStatePack *)pack)->count;
}

unsigned _supla_int_t SRPC_ICACHE_FLASH
srpc_scenestatepack_get_item_initiatorname_size(void *item) {
  return ((TSC_SuplaSceneState *)item)->InitiatorNameSize;
}

void SRPC_ICACHE_FLASH srpc_get_scene_state_pack(Tsrpc *srpc,
                                                 TsrpcReceivedData *rd) {
  srpc_getpack(
      srpc, rd, sizeof(TSC_SuplaSceneStatePack), sizeof(TSC_SuplaSceneState),
      SUPLA_SCENE_STATE_PACK_MAXCOUNT, SUPLA_INITIATOR_NAME_MAXSIZE,
      &srpc_scenestatepack_get_pack_count, &srpc_scenestatepack_set_pack_count,
      &srpc_scenestatepack_get_item_ptr,
      &srpc_scenestatepack_get_item_initiatorname_size);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_cs_async_execute_action(void *_srpc, TCS_Action *action) {
  if (!action) {
    return 0;
  }
  _supla_int_t size =
      sizeof(TCS_Action) - SUPLA_ACTION_PARAM_MAXSIZE + action->ParamSize;

  if (size > sizeof(TCS_Action)) return 0;

  return srpc_async_call(_srpc, SUPLA_CS_CALL_EXECUTE_ACTION, (char *)action,
                         size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_execute_action_with_auth(
    void *_srpc, TCS_ActionWithAuth *action) {
  if (!action) {
    return 0;
  }
  _supla_int_t size = sizeof(TCS_ActionWithAuth) - SUPLA_ACTION_PARAM_MAXSIZE +
                      action->Action.ParamSize;

  if (size > sizeof(TCS_ActionWithAuth)) return 0;

  return srpc_async_call(_srpc, SUPLA_CS_CALL_EXECUTE_ACTION_WITH_AUTH,
                         (char *)action, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_action_execution_result(
    void *_srpc, TSC_ActionExecutionResult *result) {
  if (result == NULL) {
    return 0;
  }
  return srpc_async_call(_srpc, SUPLA_SC_CALL_ACTION_EXECUTION_RESULT,
                         (char *)result, sizeof(TSC_ActionExecutionResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_get_channel_value_with_auth(
    void *_srpc, TCS_GetChannelValueWithAuth *vwa) {
  if (vwa == NULL) {
    return 0;
  }
  return srpc_async_call(_srpc, SUPLA_CS_CALL_GET_CHANNEL_VALUE_WITH_AUTH,
                         (char *)vwa, sizeof(TCS_GetChannelValueWithAuth));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_get_channel_value_result(
    void *_srpc, TSC_GetChannelValueResult *result) {
  if (result == NULL) {
    return 0;
  }
  _supla_int_t size = sizeof(TSC_GetChannelValueResult) -
                      SUPLA_CHANNELEXTENDEDVALUE_SIZE +
                      result->ExtendedValue.size;

  if (size > sizeof(TSC_GetChannelValueResult)) return 0;

  return srpc_async_call(_srpc, SUPLA_SC_CALL_GET_CHANNEL_VALUE_RESULT,
                         (char *)result, size);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_register_pn_client_token(
    void *_srpc, TCS_RegisterPnClientToken *reg) {
  if (!reg) {
    return 0;
  }

  if (reg->Token.TokenSize > SUPLA_PN_CLIENT_TOKEN_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(
      _srpc, SUPLA_CS_CALL_REGISTER_PN_CLIENT_TOKEN, (char *)reg,
      sizeof(TCS_RegisterPnClientToken) - SUPLA_PN_CLIENT_TOKEN_MAXSIZE +
          reg->Token.TokenSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_register_pn_client_token_result(
    void *_srpc, TSC_RegisterPnClientTokenResult *result) {
  if (!result) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_SC_CALL_REGISTER_PN_CLIENT_TOKEN_RESULT,
                         (char *)result,
                         sizeof(TSC_RegisterPnClientTokenResult));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_get_channel_config_request(
    void *_srpc, TCS_GetChannelConfigRequest *request) {
  if (request == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_CS_CALL_GET_CHANNEL_CONFIG,
                         (char *)request, sizeof(TCS_GetChannelConfigRequest));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_channel_config_update_or_result(
    void *_srpc, TSC_ChannelConfigUpdateOrResult *config) {
  if (config == NULL ||
      config->Config.ConfigSize > SUPLA_CHANNEL_CONFIG_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(
      _srpc, SUPLA_SC_CALL_CHANNEL_CONFIG_UPDATE_OR_RESULT, (char *)config,
      sizeof(TSC_ChannelConfigUpdateOrResult) - SUPLA_CHANNEL_CONFIG_MAXSIZE +
          config->Config.ConfigSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_set_channel_config_request(
    void *_srpc, TSCS_ChannelConfig *config) {
  if (config == NULL || config->ConfigSize > SUPLA_CHANNEL_CONFIG_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_CS_CALL_SET_CHANNEL_CONFIG,
                         (char *)config,
                         sizeof(TSCS_ChannelConfig) -
                             SUPLA_CHANNEL_CONFIG_MAXSIZE + config->ConfigSize);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_cs_async_get_device_config_request(
    void *_srpc, TCS_GetDeviceConfigRequest *request) {
  if (request == NULL) {
    return 0;
  }

  return srpc_async_call(_srpc, SUPLA_CS_CALL_GET_DEVICE_CONFIG,
                         (char *)request, sizeof(TCS_GetDeviceConfigRequest));
}

_supla_int_t SRPC_ICACHE_FLASH srpc_sc_async_device_config_update_or_result(
    void *_srpc, TSC_DeviceConfigUpdateOrResult *config) {
  if (config == NULL ||
      config->Config.ConfigSize > SUPLA_DEVICE_CONFIG_MAXSIZE) {
    return 0;
  }

  return srpc_async_call(
      _srpc, SUPLA_SC_CALL_DEVICE_CONFIG_UPDATE_OR_RESULT, (char *)config,
      sizeof(TSC_DeviceConfigUpdateOrResult) - SUPLA_DEVICE_CONFIG_MAXSIZE +
          config->Config.ConfigSize);
}

#endif /*SRPC_EXCLUDE_CLIENT*/

#ifndef SRPC_EXCLUDE_EXTENDEDVALUE_TOOLS

#ifdef USE_DEPRECATED_EMEV_V1
_supla_int_t SRPC_ICACHE_FLASH
srpc_evtool_v1_emextended2extended(const TElectricityMeter_ExtendedValue *em_ev,
                                   TSuplaChannelExtendedValue *ev) {
  if (em_ev == NULL || ev == NULL || em_ev->m_count > EM_MEASUREMENT_COUNT ||
      em_ev->m_count < 0) {
    return 0;
  }

  memset(ev, 0, sizeof(TSuplaChannelExtendedValue));
  ev->type = EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1;

  ev->size = sizeof(TElectricityMeter_ExtendedValue) -
             sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT +
             sizeof(TElectricityMeter_Measurement) * em_ev->m_count;

  if (ev->size > 0 && ev->size <= SUPLA_CHANNELEXTENDEDVALUE_SIZE) {
    memcpy(ev->value, em_ev, ev->size);
    return 1;
  }

  ev->size = 0;
  return 0;
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_evtool_v1_extended2emextended(const TSuplaChannelExtendedValue *ev,
                                   TElectricityMeter_ExtendedValue *em_ev) {
  if (em_ev == NULL || ev == NULL ||
      ev->type != EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1 || ev->size == 0 ||
      ev->size > sizeof(TElectricityMeter_ExtendedValue)) {
    return 0;
  }

  memset(em_ev, 0, sizeof(TElectricityMeter_ExtendedValue));
  memcpy(em_ev, ev->value, ev->size);

  _supla_int_t expected_size = 0;

  if (em_ev->m_count <= EM_MEASUREMENT_COUNT) {
    expected_size =
        sizeof(TElectricityMeter_ExtendedValue) -
        sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT +
        sizeof(TElectricityMeter_Measurement) * em_ev->m_count;
  }

  if (ev->size != expected_size) {
    memset(em_ev, 0, sizeof(TElectricityMeter_ExtendedValue));
    return 0;
  }

  return 1;
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_evtool_emev_v1to2(const TElectricityMeter_ExtendedValue *v1,
                       TElectricityMeter_ExtendedValue_V2 *v2) {
  if (v1 == NULL || v2 == NULL) {
    return 0;
  }
  memset(v2, 0, sizeof(TElectricityMeter_ExtendedValue_V2));

  for (int a = 0; a < 3; a++) {
    v2->total_forward_active_energy[a] = v1->total_forward_active_energy[a];
    v2->total_reverse_active_energy[a] = v1->total_reverse_active_energy[a];
    v2->total_forward_reactive_energy[a] = v1->total_forward_reactive_energy[a];
    v2->total_reverse_reactive_energy[a] = v1->total_reverse_reactive_energy[a];
  }

  v2->total_cost = v1->total_cost;
  v2->price_per_unit = v1->price_per_unit;
  memcpy(v2->currency, v1->currency, sizeof(v2->currency));
  v2->measured_values = v1->measured_values;
  v2->period = v1->period;
  v2->m_count = v1->m_count;

  memcpy(v2->m, v1->m,
         sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT);

  v2->measured_values ^=
      v1->measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;
  v2->measured_values ^=
      v1->measured_values & EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED;

  return 1;
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_evtool_emev_v2to1(const TElectricityMeter_ExtendedValue_V2 *v2,
                       TElectricityMeter_ExtendedValue *v1) {
  if (v1 == NULL || v2 == NULL) {
    return 0;
  }
  memset(v1, 0, sizeof(TElectricityMeter_ExtendedValue));

  for (int a = 0; a < 3; a++) {
    v1->total_forward_active_energy[a] = v2->total_forward_active_energy[a];
    v1->total_reverse_active_energy[a] = v2->total_reverse_active_energy[a];
    v1->total_forward_reactive_energy[a] = v2->total_forward_reactive_energy[a];
    v1->total_reverse_reactive_energy[a] = v2->total_reverse_reactive_energy[a];
  }

  v1->total_cost = v2->total_cost;
  v1->price_per_unit = v2->price_per_unit;
  memcpy(v1->currency, v2->currency, sizeof(v2->currency));
  v1->measured_values = v2->measured_values;
  v1->period = v2->period;
  v1->m_count = v2->m_count;

  memcpy(v1->m, v2->m,
         sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT);

  v1->measured_values ^=
      v1->measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED;
  v1->measured_values ^=
      v1->measured_values & EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED;

  return 1;
}

#endif /*USE_DEPRECATED_EMEV_V1*/

_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v2_emextended2extended(
    const TElectricityMeter_ExtendedValue_V2 *em_ev,
    TSuplaChannelExtendedValue *ev) {
  if (em_ev == NULL || ev == NULL || em_ev->m_count > EM_MEASUREMENT_COUNT ||
      em_ev->m_count < 0) {
    return 0;
  }

  memset(ev, 0, sizeof(TSuplaChannelExtendedValue));
  ev->type = EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2;

  ev->size = sizeof(TElectricityMeter_ExtendedValue_V2) -
             sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT +
             sizeof(TElectricityMeter_Measurement) * em_ev->m_count;

  if (ev->size > 0 && ev->size <= SUPLA_CHANNELEXTENDEDVALUE_SIZE) {
    memcpy(ev->value, em_ev, ev->size);
    return 1;
  }

  ev->size = 0;
  return 0;
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_evtool_v2_extended2emextended(const TSuplaChannelExtendedValue *ev,
                                   TElectricityMeter_ExtendedValue_V2 *em_ev) {
  if (em_ev == NULL || ev == NULL ||
      ev->type != EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2 || ev->size == 0 ||
      ev->size > sizeof(TElectricityMeter_ExtendedValue_V2)) {
    return 0;
  }

  memset(em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V2));
  memcpy(em_ev, ev->value, ev->size);

  _supla_int_t expected_size = 0;

  if (em_ev->m_count <= EM_MEASUREMENT_COUNT) {
    expected_size =
        sizeof(TElectricityMeter_ExtendedValue_V2) -
        sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT +
        sizeof(TElectricityMeter_Measurement) * em_ev->m_count;
  }

  if (ev->size != expected_size) {
    memset(em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V2));
    return 0;
  }

  return 1;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_extended2thermostatextended(
    const TSuplaChannelExtendedValue *ev, TThermostat_ExtendedValue *th_ev) {
  if (ev == NULL || th_ev == NULL ||
      ev->type != EV_TYPE_THERMOSTAT_DETAILS_V1 || ev->size == 0 ||
      ev->size > sizeof(TThermostat_ExtendedValue)) {
    return 0;
  }

  memset(th_ev, 0, sizeof(TThermostat_ExtendedValue));
  memcpy(th_ev, ev->value, ev->size);

  return 1;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_thermostatextended2extended(
    const TThermostat_ExtendedValue *th_ev, TSuplaChannelExtendedValue *ev) {
  if (th_ev == NULL || ev == NULL) {
    return 0;
  }

  memset(ev, 0, sizeof(TSuplaChannelExtendedValue));
  ev->type = EV_TYPE_THERMOSTAT_DETAILS_V1;
  ev->size = 0;

  unsigned _supla_int_t size = sizeof(TThermostat_ExtendedValue);

  if (0 == (th_ev->Fields & THERMOSTAT_FIELD_Schedule)) {
    size -= sizeof(th_ev->Schedule);
    if (0 == (th_ev->Fields & THERMOSTAT_FIELD_Time)) {
      size -= sizeof(th_ev->Time);
      if (0 == (th_ev->Fields & THERMOSTAT_FIELD_Values)) {
        size -= sizeof(th_ev->Values);
        if (0 == (th_ev->Fields & THERMOSTAT_FIELD_Flags)) {
          size -= sizeof(th_ev->Flags);
          if (0 == (th_ev->Fields & THERMOSTAT_FIELD_PresetTemperatures)) {
            size -= sizeof(th_ev->PresetTemperature);
            if (0 == (th_ev->Fields & THERMOSTAT_FIELD_MeasuredTemperatures)) {
              size -= sizeof(th_ev->MeasuredTemperature);
            }
          }
        }
      }
    }
  }

  if (size > 0) {
    ev->size = size;
    memcpy(ev->value, th_ev, size);
    return 1;
  }

  return 0;
}

#ifndef SRPC_EXCLUDE_CLIENT
_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_v1_icextended2extended(
    const TSC_ImpulseCounter_ExtendedValue *ic_ev,
    TSuplaChannelExtendedValue *ev) {
  if (ic_ev == NULL || ev == NULL) {
    return 0;
  }

  memset(ev, 0, sizeof(TSuplaChannelExtendedValue));
  ev->type = EV_TYPE_IMPULSE_COUNTER_DETAILS_V1;
  ev->size = sizeof(TSC_ImpulseCounter_ExtendedValue);

  memcpy(ev->value, ic_ev, ev->size);
  return 1;
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_evtool_v1_extended2icextended(const TSuplaChannelExtendedValue *ev,
                                   TSC_ImpulseCounter_ExtendedValue *ic_ev) {
  if (ic_ev == NULL || ev == NULL ||
      ev->type != EV_TYPE_IMPULSE_COUNTER_DETAILS_V1 || ev->size == 0 ||
      ev->size != sizeof(TSC_ImpulseCounter_ExtendedValue)) {
    return 0;
  }

  memset(ic_ev, 0, sizeof(TSC_ImpulseCounter_ExtendedValue));
  memcpy(ic_ev, ev->value, ev->size);

  return 1;
}

_supla_int_t SRPC_ICACHE_FLASH srpc_evtool_value_add(
    TSuplaChannelExtendedValue *dest, TSuplaChannelExtendedValue *src) {
  if (!dest || !src) {
    return 0;
  }

  if (src->type == EV_TYPE_NONE || src->size == 0) {
    return 0;
  }

  if (dest->type == EV_TYPE_NONE) {
    memcpy(dest, src, sizeof(TSuplaChannelExtendedValue));
    return 1;
  }

  unsigned int src_size =
      sizeof(TSuplaChannelExtendedValue) - (sizeof(src->value) - src->size);

  if (dest->type == EV_TYPE_MULTI_VALUE) {
    if (dest->size + src_size > sizeof(dest->value)) {
      return 0;
    }

    memcpy(&dest->value[dest->size], src, src_size);
    dest->size += src_size;
    return 1;
  }

  unsigned int prev_size =
      sizeof(TSuplaChannelExtendedValue) - (sizeof(dest->value) - dest->size);

  if (prev_size + src_size > sizeof(dest->value)) {
    return 0;
  }

  TSuplaChannelExtendedValue prev = {};
  memcpy(&prev, dest, sizeof(TSuplaChannelExtendedValue));

  dest->type = EV_TYPE_MULTI_VALUE;
  dest->size = prev_size;

  memcpy(dest->value, &prev, prev_size);

  return srpc_evtool_value_add(dest, src);
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_evtool_value_get(TSuplaChannelExtendedValue *ev, unsigned short index,
                      TSuplaChannelExtendedValue *dest) {
  if (dest == NULL || ev == NULL || ev->type == EV_TYPE_NONE) {
    return 0;
  }

  if (ev->type == EV_TYPE_MULTI_VALUE) {
    unsigned int header_size =
        sizeof(TSuplaChannelExtendedValue) - sizeof(ev->value);
    unsigned int offset = 0;
    unsigned int size = ev->size;
    if (size > sizeof(ev->value)) {
      size = sizeof(ev->value);
    }
    size -= header_size;

    unsigned short n = 0;

    while (offset < size) {
      TSuplaChannelExtendedValue *next =
          (TSuplaChannelExtendedValue *)&ev->value[offset];

      if (next->type == EV_TYPE_NONE || next->size == 0 ||
          next->size + offset + header_size > sizeof(ev->value)) {
        return 0;
      }

      if (n == index) {
        memcpy(dest, next, header_size + next->size);
        return 1;
      }

      offset += header_size + next->size;
      n++;
    }
  }

  if (index == 0) {
    memcpy(dest, ev, sizeof(TSuplaChannelExtendedValue));
    return 1;
  }

  return 0;
}

#endif /*SRPC_EXCLUDE_CLIENT*/

#endif /*SRPC_EXCLUDE_EXTENDEDVALUE_TOOLS*/
