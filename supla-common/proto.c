/*
 ============================================================================
 Name        : proto.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.3
 Copyright   : 2015-2016 GPLv2
 ============================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "proto.h"
#include "log.h"

#ifdef ESP8266

	#include <osapi.h>
	#include <mem.h>
	
	#define BUFFER_MIN_SIZE    512
	#define BUFFER_MAX_SIZE    2048
	
	#define malloc os_malloc
	#define free os_free
	#define realloc os_realloc

#elif defined(__AVR__)

	#define BUFFER_MIN_SIZE    32
	#define BUFFER_MAX_SIZE    1024

#else

	#define BUFFER_MIN_SIZE    0
	//sizeof(TSuplaDataPacket)
	#define BUFFER_MAX_SIZE    131072

#endif

static char sproto_tag[SUPLA_TAG_SIZE] = { 'S', 'U', 'P', 'L', 'A' };

typedef struct {

	unsigned char begin_tag;
	unsigned _supla_int_t size;
	unsigned _supla_int_t data_size;

	char *buffer;

}TSuplaProtoInBuffer;

typedef struct {

	unsigned _supla_int_t size;
	unsigned _supla_int_t data_size;

	char *buffer;

}TSuplaProtoOutBuffer;

typedef struct {

	unsigned _supla_int_t next_rr_id;
	unsigned char version;
	TSuplaProtoInBuffer in;
	TSuplaProtoOutBuffer out;

}TSuplaProtoData;


void *sproto_init(void) {
	TSuplaProtoData *spd = malloc(sizeof(TSuplaProtoData));
	if ( spd ) {
		memset(spd, 0, sizeof(TSuplaProtoData));
		spd->version = SUPLA_PROTO_VERSION;
		return (spd);
	}

	return (NULL);
}

void sproto_free(void *spd_ptr) {

	TSuplaProtoData *spd = (TSuplaProtoData *)spd_ptr;
	if ( spd != NULL ) {
       if ( spd->in.buffer != NULL ) free(spd->in.buffer);
       if ( spd->out.buffer != NULL ) free(spd->out.buffer);

       free(spd);
	}

}

unsigned char sproto_buffer_append(void *spd_ptr, char **buffer, unsigned _supla_int_t *buffer_size,
		unsigned _supla_int_t *buffer_data_size, char *data, unsigned _supla_int_t data_size) {

	unsigned _supla_int_t size = *buffer_size;

	if ( size < BUFFER_MIN_SIZE ) {
       size = BUFFER_MIN_SIZE;
	}


	if ( data_size > size-(*buffer_data_size) )  {
       size += data_size - (size - (*buffer_data_size));
	}

	if ( size >= BUFFER_MAX_SIZE ) 
		return (SUPLA_RESULT_BUFFER_OVERFLOW);
		
	if ( size != (*buffer_size) )
		*buffer = (char *)realloc(*buffer, size);

    #ifndef ESP8266
    #ifndef __AVR__
    if ( errno == ENOMEM )
    	return (SUPLA_RESULT_FALSE);
    #endif
    #endif

    memcpy(&(*buffer)[(*buffer_data_size)], data, data_size);

    (*buffer_size)=size;
    (*buffer_data_size)+=data_size;

	return (SUPLA_RESULT_TRUE);
}

char sproto_in_buffer_append(void *spd_ptr, char *data, unsigned _supla_int_t data_size) {

	TSuplaProtoData *spd = (TSuplaProtoData *)spd_ptr;
    return sproto_buffer_append(spd_ptr, &spd->in.buffer, &spd->in.size, &spd->in.data_size, data, data_size);
}

char sproto_out_buffer_append(void *spd_ptr, TSuplaDataPacket *sdp) {

	TSuplaProtoData *spd = (TSuplaProtoData *)spd_ptr;
	unsigned _supla_int_t sdp_size = sizeof(TSuplaDataPacket);
	unsigned _supla_int_t packet_size = sdp_size-SUPLA_MAX_DATA_SIZE+sdp->data_size;

	if ( packet_size+SUPLA_TAG_SIZE > sdp_size )
		return SUPLA_RESULT_DATA_TOO_LARGE;

	if ( SUPLA_RESULT_TRUE == sproto_buffer_append(spd_ptr, &spd->out.buffer, &spd->out.size, &spd->out.data_size, (char*)sdp, packet_size) ) {
		return sproto_buffer_append(spd_ptr, &spd->out.buffer, &spd->out.size, &spd->out.data_size, sproto_tag, SUPLA_TAG_SIZE);
	}

	return (SUPLA_RESULT_FALSE);
}

unsigned _supla_int_t sproto_pop_out_data(void *spd_ptr, char *buffer, unsigned _supla_int_t buffer_size) {

	unsigned _supla_int_t a;
	unsigned _supla_int_t b;

	TSuplaProtoData *spd = (TSuplaProtoData *)spd_ptr;

    if ( spd->out.data_size <= 0
    	   || buffer_size == 0
    	   || buffer == NULL )
    	return (0);

    if ( spd->out.data_size < buffer_size )
    	buffer_size = spd->out.data_size;


    memcpy(buffer, spd->out.buffer, buffer_size);

    b=0;

    for(a=buffer_size; a < spd->out.data_size; a++) {
    	spd->out.buffer[b] = spd->out.buffer[a];
    	b++;
    }

    spd->out.data_size-=buffer_size;

    if ( spd->out.data_size < spd->out.size ) {

    	b = spd->out.size;

    	spd->out.size = spd->out.data_size;
    	if ( spd->out.size < BUFFER_MIN_SIZE )
    		spd->out.size = BUFFER_MIN_SIZE;

    	if ( b != spd->out.size )
    	  spd->out.buffer = (char *)realloc(spd->out.buffer, spd->out.size);
    }

    return (buffer_size);
}

char sproto_out_dataexists(void *spd_ptr) {
	return ((TSuplaProtoData *)spd_ptr)->out.data_size > 0 ? SUPLA_RESULT_TRUE : SUPLA_RESULT_FALSE;
}

void sproto_shrink_in_buffer(TSuplaProtoInBuffer *in, unsigned _supla_int_t size) {

	unsigned _supla_int_t old_size = in->size;
	_supla_int_t a, b;

	in->begin_tag = 0;

	if ( size > in->data_size )
		size = in->data_size;

    b=0;

    for(a=size; a < in->data_size; a++) {
    	in->buffer[b] = in->buffer[a];
    	b++;
    }

	in->data_size-=size;


	if ( in->data_size < in->size ) {
		in->size = in->data_size;

		if ( in->size < BUFFER_MIN_SIZE )
			in->size = BUFFER_MIN_SIZE;

		if ( old_size != in->size )
			in->buffer = (char *)realloc(in->buffer, in->size);
	}

}

char sproto_pop_in_sdp(void *spd_ptr, TSuplaDataPacket *sdp) {

	unsigned _supla_int_t header_size;
	TSuplaDataPacket *_sdp;

	TSuplaProtoData *spd = (TSuplaProtoData *)spd_ptr;

	if ( spd->in.begin_tag == 0
		 && spd->in.data_size >= SUPLA_TAG_SIZE )  {

			if ( memcmp(spd->in.buffer, sproto_tag, SUPLA_TAG_SIZE) == 0 ) {
				spd->in.begin_tag = 1;
			} else {				
				sproto_shrink_in_buffer(&spd->in, spd->in.data_size);
				return SUPLA_RESULT_DATA_ERROR;
			}

	}


	if ( spd->in.begin_tag == 1 ) {

		header_size = sizeof(TSuplaDataPacket)-SUPLA_MAX_DATA_SIZE;
		if ( (spd->in.data_size-SUPLA_TAG_SIZE) >= header_size ) {

			_sdp = (TSuplaDataPacket *)spd->in.buffer;

			if ( _sdp->version > SUPLA_PROTO_VERSION
					|| _sdp->version < SUPLA_PROTO_VERSION_MIN ) {

				sdp->version = _sdp->version;
				sproto_shrink_in_buffer(&spd->in, spd->in.data_size);

				return SUPLA_RESULT_VERSION_ERROR;
			}

			if ( (header_size+_sdp->data_size+SUPLA_TAG_SIZE) > sizeof(TSuplaDataPacket) ) {
				
				sproto_shrink_in_buffer(&spd->in, spd->in.data_size);
				return SUPLA_RESULT_DATA_ERROR;
			}

			if ( (header_size+_sdp->data_size+SUPLA_TAG_SIZE) > spd->in.data_size ) 
				return SUPLA_RESULT_FALSE;		


			if ( memcmp(&spd->in.buffer[header_size+_sdp->data_size], sproto_tag, SUPLA_TAG_SIZE) != 0 ) {

				sproto_shrink_in_buffer(&spd->in, spd->in.data_size);

				return SUPLA_RESULT_DATA_ERROR;
			}

			memcpy(sdp, spd->in.buffer, header_size+_sdp->data_size);
			sproto_shrink_in_buffer(&spd->in, header_size+_sdp->data_size+SUPLA_TAG_SIZE);

			return (SUPLA_RESULT_TRUE);
		}

	};

	return (SUPLA_RESULT_FALSE);
}

void sproto_set_version(void *spd_ptr, unsigned char version) {
	((TSuplaProtoData *)spd_ptr)->version = version;
}

unsigned char sproto_get_version(void *spd_ptr) {
	return ((TSuplaProtoData *)spd_ptr)->version;
}

void sproto_sdp_init(void *spd_ptr, TSuplaDataPacket *sdp) {

	TSuplaProtoData *spd = (TSuplaProtoData *)spd_ptr;

	memset(sdp, 0, sizeof(TSuplaDataPacket));
	memcpy(sdp->tag, sproto_tag, SUPLA_TAG_SIZE);

	spd->next_rr_id++;

	if ( spd->next_rr_id == 0 )
		spd->next_rr_id++;

	sdp->rr_id = spd->next_rr_id;
	sdp->version = spd->version;
}

TSuplaDataPacket* sproto_sdp_malloc(void *spd_ptr) {

	TSuplaDataPacket *result = malloc(sizeof(TSuplaDataPacket));

	if ( result )
    	sproto_sdp_init(spd_ptr, result);

	return result;
}

void sproto_sdp_free(TSuplaDataPacket* sdp) {
    free(sdp);
}

char sproto_set_data(TSuplaDataPacket *sdp, char *data, unsigned _supla_int_t data_size, unsigned _supla_int_t call_type) {

	if ( data_size >  SUPLA_MAX_DATA_SIZE )
		return SUPLA_RESULT_FALSE;


	if ( data_size > 0 )
	  memcpy(sdp->data, data, data_size);

	sdp->data_size = data_size;
    sdp->call_type = call_type;
	return SUPLA_RESULT_TRUE;
}

void sproto_log_summary(void *spd_ptr) {

	if ( spd_ptr == NULL ) {
		supla_log(LOG_DEBUG, "SPROTO - Not initialized!");
		return;
	}

	TSuplaProtoData *spd = (TSuplaProtoData *)spd_ptr;

    supla_log(LOG_DEBUG, "BUFFER IN");
    supla_log(LOG_DEBUG, "         size: %i", spd->in.size);
    supla_log(LOG_DEBUG, "    data_size: %i", spd->in.data_size);
    supla_log(LOG_DEBUG, "    begin_tag: %i", spd->in.begin_tag);

    supla_log(LOG_DEBUG, "BUFFER OUT");
    supla_log(LOG_DEBUG, "         size: %i", spd->out.size);
    supla_log(LOG_DEBUG, "    data_size: %i", spd->out.data_size);

}

void sproto_buffer_dump(void *spd_ptr, unsigned char in) {

	_supla_int_t a;
	char *buffer;
	_supla_int_t size;

	TSuplaProtoData *spd = (TSuplaProtoData *)spd_ptr;

	if ( in != 0 ) {
		buffer = spd->in.buffer;
		size = spd->in.data_size;
	} else {
		buffer = spd->out.buffer;
		size = spd->out.data_size;
	}

	for(a=0;a<size;a++)
		supla_log(LOG_DEBUG, "%c [%i]", buffer[a], buffer[a]);


}

