/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __DMMSG_H
#define __DMMSG_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <arpa/inet.h>
#include <netinet/in.h>
#include <bits/byteswap.h>
#include <stdint.h>
#include <string.h>

/* see http://www.iana.org/assignments/address-family-numbers/address-family-numbers.txt */
#define IANA_INET      1
#define IANA_INET6     2

#if __BYTE_ORDER == __BIG_ENDIAN
/* The host byte order is the same as network byte order,
   so these functions are all just identity.  */
#define ntohll(x)		(x)
#define htonll(x)		(x)

#else

#define ntohll(x)		__bswap_64(x)
#define htonll(x)		__bswap_64(x)
#endif

#define DM_BLOCK_ALLOC	2048

#define PAD32(x)		((x + 3) & ~3)

#define TIME_SINCE_EPOCH	2208988800

		/* structures */
typedef struct dm_request		DM_REQUEST;
typedef struct dm_avp			DM_AVP;
typedef struct dm_avpgrp_info		DM_AVPGRP_INFO;
typedef struct dm_avpgrp		DM_AVPGRP;
typedef struct dm_request_info	DM_REQUEST_INFO;
typedef struct dm_packet		DM_PACKET;
typedef struct dm_request_head	DM_REQUEST_HEAD;
typedef struct dm_timeval		DM_TIMEVAL;

struct dm_avp {
	uint32_t	code;
	uint8_t		flags;
	uint8_t		length[3];
	uint32_t	vendor_id;
} __attribute__ ((packed));

struct dm_avpgrp_info {
	DM_AVP	*avpptr;
	uint32_t	size;
	uint32_t	avps_length;
} __attribute__ ((packed));

struct dm_avpgrp {
	DM_AVPGRP_INFO info;

	/* AVP group follows */
} __attribute__ ((packed));

struct dm_request_info {
	DM_REQUEST	*next;
	DM_AVP	*avpptr;
	uint32_t	size;
} __attribute__ ((packed));

struct dm_packet {
	uint8_t		version;
	uint8_t		length[3];
	uint8_t		flags;
	uint8_t		code[3];
	uint32_t	app_id;
	uint32_t	hop2hop_id;
	uint32_t	end2end_id;
} __attribute__ ((packed));

struct dm_request {
	DM_REQUEST_INFO	info;
	DM_PACKET		packet;
	/* AVP group follows */
} __attribute__ ((packed));

	/* timeval structure as transmitted in an AVP_TIMEVAL */
struct dm_timeval {
	uint32_t	tv_sec;		/* maximum size of both fields is 32 bit */
	uint32_t	tv_usec;
} __attribute__ ((packed));

		/* dmconfig packet flag bitmask constants */

#define CMD_FLAG_REQUEST	(1 << 7)
#define CMD_FLAG_PROXIABLE	(1 << 6)
#define CMD_FLAG_ERROR		(1 << 5)
#define CMD_FLAG_RETRANSMITED	(1 << 4)
#define CMD_FLAG_RESERVED	(~(CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE | CMD_FLAG_ERROR | CMD_FLAG_RETRANSMITED))

		/* dmconfig AVP flag bitmask constants */

#define AVP_FLAG_VENDOR		(1 << 7)
#define AVP_FLAG_MANDATORY	(1 << 6)
#define AVP_FLAG_PRIVAT		(1 << 5)
#define AVP_FLAG_RESERVED	(~(AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY | AVP_FLAG_PRIVAT))

		/* function headers */

int get_avp(uint32_t *code, uint8_t *flags, uint32_t *vendor_id,
	    void **data, size_t *len, DM_AVP **src);

DM_REQUEST *new_dm_request(void *ctx, uint32_t code, uint8_t flags,
			       uint32_t appid, uint32_t hopid, uint32_t endid);
int dm_decode_request(void *ctx, DM_REQUEST **req, void *data, size_t len);
int dm_add_data(DM_REQUEST *req, void *data, size_t len);
int dm_decode_complete(DM_REQUEST *req);

int build_dm_request(void *ctx, DM_REQUEST **req, DM_AVPGRP *avpgrp);
DM_AVPGRP *new_dm_avpgrp(void *ctx);
DM_AVPGRP *dm_decode_avpgrp(void *ctx, void *data, size_t len);
int dm_avpgrp_add_avpgrp(void *ctx, DM_AVPGRP **avpgrp,
			   uint32_t code, uint8_t flags, uint32_t vendor_id,
			   DM_AVPGRP *source);
int dm_avpgrp_insert_raw(void *ctx, DM_AVPGRP **avpgrp, const void *data, size_t len);
int dm_avpgrp_add_raw(void *ctx, DM_AVPGRP **avpgrp,
			uint32_t code, uint8_t flags, uint32_t vendor_id,
			const void *data, size_t len);
int dm_avpgrp_add_address(void *ctx, DM_AVPGRP **avpgrp,
			    uint32_t code, uint8_t flags, uint32_t vendor_id,
			    int af, const void *data);
int dm_avpgrp_add_uint32_string(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
				  uint8_t flags, uint32_t vendor_id, uint32_t d1, const char *d2);
int dm_get_address_avp(int *af, void *addr, socklen_t size, const void *src, size_t len);

static inline uint32_t uint24to32(uint8_t i24[3]);
static inline void uint32to24(uint8_t i24[3], uint32_t i32);
static inline uint8_t dm_packet_flags(DM_PACKET *pkt);
static inline uint32_t dm_packet_length(DM_PACKET *pkt);
static inline uint32_t dm_packet_code(DM_PACKET *pkt);
static inline uint32_t dm_app_id(DM_PACKET *pkt);
static inline uint32_t dm_end2end_id(DM_PACKET *pkt);
static inline uint32_t dm_hop2hop_id(DM_PACKET *pkt);
static inline uint32_t dm_avp_length(DM_AVP *avp);

static inline void dm_request_reset_avp(DM_REQUEST *req);
static inline int dm_request_get_avp(DM_REQUEST *req, uint32_t *code,
				       uint8_t *flags, uint32_t *vendor_id, void **data, size_t *len);

static inline void dm_avpgrp_reset_avp(DM_AVPGRP *avpgrp);
static inline int dm_avpgrp_get_avp(DM_AVPGRP *avpgrp, uint32_t *code, uint8_t *flags,
				      uint32_t *vendor_id, void **data, size_t *len);
static inline int dm_avpgrp_add_string(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					 uint8_t flags, uint32_t vendor_id, const char *data);
static inline int dm_avpgrp_add_int8(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
				       uint8_t flags, uint32_t vendor_id, int8_t data);
static inline int dm_avpgrp_add_uint8(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					uint8_t flags, uint32_t vendor_id, uint8_t data);
static inline int dm_avpgrp_add_int16(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					uint8_t flags, uint32_t vendor_id, int16_t data);
static inline int dm_avpgrp_add_uint16(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					 uint8_t flags, uint32_t vendor_id, uint16_t data);
static inline int dm_avpgrp_add_int32(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					uint8_t flags, uint32_t vendor_id, int32_t data);
static inline int dm_avpgrp_add_uint32(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					 uint8_t flags, uint32_t vendor_id, uint32_t data);
static inline int dm_avpgrp_add_uint64(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					 uint8_t flags, uint32_t vendor_id, uint64_t data);
static inline int dm_avpgrp_add_int64(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					uint8_t flags, uint32_t vendor_id, int64_t data);
static inline int dm_avpgrp_add_float32(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					  uint8_t flags, uint32_t vendor_id, float data);
static inline int dm_avpgrp_add_float64(void *ctx, DM_AVPGRP **avpgrp, uint32_t code,
					  uint8_t flags, uint32_t vendor_id, double data);
static inline int dm_avpgrp_add_time(void *ctx, DM_AVPGRP **avpgrp, uint32_t code, uint8_t flags,
				       uint32_t vendor_id, time_t data);
static inline int dm_avpgrp_add_timeval(void *ctx, DM_AVPGRP **avpgrp,
					  uint32_t code, uint8_t flags, uint32_t vendor_id,
					  struct timeval value);
static inline int dm_avpgrp_insert_avpgrp(void *ctx, DM_AVPGRP **avpgrp, DM_AVPGRP *source);

static inline int dm_get_string_avp(char *dest, size_t dlen, const void *src, size_t slen);
static inline int8_t dm_get_int8_avp(const void *src);
static inline uint8_t dm_get_uint8_avp(const void *src);
static inline int16_t dm_get_int16_avp(const void *src);
static inline uint16_t dm_get_uint16_avp(const void *src);
static inline int32_t dm_get_int32_avp(const void *src);
static inline uint32_t dm_get_uint32_avp(const void *src);
static inline int64_t dm_get_int64_avp(const void *src);
static inline uint64_t dm_get_uint64_avp(const void *src);
static inline time_t dm_get_time_avp(const void *src);
static inline struct timeval dm_get_timeval_avp(const void *src);

		/* inline functions */

		/* operating on dmconfig and AVP headers */

static inline uint32_t
uint24to32(uint8_t i24[3]) {
	return i24[0] << 16 | i24[1] << 8 | i24[2];
}

static inline void
uint32to24(uint8_t i24[3], uint32_t i32) {
	i24[2] = i32 & 0xff;
	i24[1] = (i32 >>  8) & 0xff;
	i24[0] = (i32 >> 16) & 0xff;
}

static inline uint8_t
dm_packet_flags(DM_PACKET *pkt) {
	return pkt->flags;
}

static inline uint32_t
dm_packet_length(DM_PACKET *pkt) {
	return uint24to32(pkt->length);
}

static inline uint32_t
dm_packet_code(DM_PACKET *pkt) {
	return uint24to32(pkt->code);
}

static inline uint32_t
dm_app_id(DM_PACKET *pkt) {
	return ntohl(pkt->app_id);
}

static inline uint32_t
dm_end2end_id(DM_PACKET *pkt) {
	return ntohl(pkt->end2end_id);
}

static inline uint32_t
dm_hop2hop_id(DM_PACKET *pkt) {
	return ntohl(pkt->hop2hop_id);
}

static inline uint32_t
dm_avp_length(DM_AVP *avp) {
	return uint24to32(avp->length);
}

		/* operating on request and AVP group objects */

static inline void
dm_request_reset_avp(DM_REQUEST *req) {
	req->info.avpptr = (DM_AVP*)((uint8_t*)req + sizeof(DM_REQUEST));
}

static inline int
dm_request_get_avp(DM_REQUEST *req,
		     uint32_t *code, uint8_t *flags, uint32_t *vendor_id,
		     void **data, size_t *len) {
	return req->info.avpptr < (DM_AVP*)((uint8_t*)req + req->info.size) ?
	       get_avp(code, flags, vendor_id, data, len, &req->info.avpptr) : 1;
}

static inline void
dm_avpgrp_reset_avp(DM_AVPGRP *avpgrp) {
	avpgrp->info.avpptr = (DM_AVP*)((uint8_t*)avpgrp + sizeof(DM_AVPGRP));
}

static inline int
dm_avpgrp_get_avp(DM_AVPGRP *avpgrp,
		    uint32_t *code, uint8_t *flags, uint32_t *vendor_id,
		    void **data, size_t *len) {
	return avpgrp->info.avpptr < (DM_AVP*)((uint8_t*)avpgrp + sizeof(DM_AVPGRP) +
	       avpgrp->info.avps_length) ? get_avp(code, flags, vendor_id, data, len, &avpgrp->info.avpptr) : 1;
}

static inline int
dm_avpgrp_add_string(void *ctx, DM_AVPGRP **avpgrp,
		       uint32_t code, uint8_t flags, uint32_t vendor_id,
		       const char *data) {
	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, data, strlen(data));
}

static inline int
dm_avpgrp_add_int8(void *ctx, DM_AVPGRP **avpgrp,
		     uint32_t code, uint8_t flags, uint32_t vendor_id,
		     int8_t data) {
	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &data, sizeof(data));
}

static inline int
dm_avpgrp_add_uint8(void *ctx, DM_AVPGRP **avpgrp,
		      uint32_t code, uint8_t flags, uint32_t vendor_id,
		      uint8_t data) {
	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &data, sizeof(data));
}

static inline int
dm_avpgrp_add_int16(void *ctx, DM_AVPGRP **avpgrp,
		      uint32_t code, uint8_t flags, uint32_t vendor_id,
		      int16_t data) {
	int16_t val = htons(data);

	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &val, sizeof(val));
}

static inline int
dm_avpgrp_add_uint16(void *ctx, DM_AVPGRP **avpgrp,
		       uint32_t code, uint8_t flags, uint32_t vendor_id,
		       uint16_t data) {
	uint16_t val = htons(data);

	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &val, sizeof(val));
}

static inline int
dm_avpgrp_add_int32(void *ctx, DM_AVPGRP **avpgrp,
		      uint32_t code, uint8_t flags, uint32_t vendor_id,
		      int32_t data) {
	int32_t val = htonl(data);

	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &val, sizeof(val));
}

static inline int
dm_avpgrp_add_uint32(void *ctx, DM_AVPGRP **avpgrp,
		       uint32_t code, uint8_t flags, uint32_t vendor_id,
		       uint32_t data) {
	uint32_t val = htonl(data);

	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &val, sizeof(val));
}

static inline int
dm_avpgrp_add_uint64(void *ctx, DM_AVPGRP **avpgrp,
		       uint32_t code, uint8_t flags, uint32_t vendor_id,
		       uint64_t data) {
	uint64_t val = htonll(data);

	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &val, sizeof(val));
}

static inline int
dm_avpgrp_add_int64(void *ctx, DM_AVPGRP **avpgrp,
		      uint32_t code, uint8_t flags, uint32_t vendor_id,
		      int64_t data) {
	int64_t val = htonll(data);

	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &val, sizeof(val));
}

static inline int
dm_avpgrp_add_float32(void *ctx, DM_AVPGRP **avpgrp,
			uint32_t code, uint8_t flags, uint32_t vendor_id,
			float data) {
	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &data, sizeof(data));
}

static inline int
dm_avpgrp_add_float64(void *ctx, DM_AVPGRP **avpgrp,
			uint32_t code, uint8_t flags, uint32_t vendor_id,
			double data) {
	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &data, sizeof(data));
}

static inline int
dm_avpgrp_add_time(void *ctx, DM_AVPGRP **avpgrp,
		     uint32_t code, uint8_t flags, uint32_t vendor_id,
		     time_t data) {
	uint32_t dtime;

	if(data + TIME_SINCE_EPOCH > 0xFFFFFFFF)
		return 1;

	dtime = htonl((uint32_t)data + TIME_SINCE_EPOCH);
	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &dtime, sizeof(uint32_t));
}

		/* accept only timevals with both fields <= 32 bits (standardization) */
static inline int
dm_avpgrp_add_timeval(void *ctx, DM_AVPGRP **avpgrp,
			uint32_t code, uint8_t flags, uint32_t vendor_id,
			struct timeval value) {
	DM_TIMEVAL tvalue;

	if(value.tv_sec > (int32_t)0x7FFFFFFF || value.tv_usec >= 1000000)
		return 1;

	tvalue.tv_sec = htonl((uint32_t)value.tv_sec);
	tvalue.tv_usec = htonl((uint32_t)value.tv_usec);

	return dm_avpgrp_add_raw(ctx, avpgrp, code, flags, vendor_id, &tvalue, sizeof(DM_TIMEVAL));
}

static inline int
dm_avpgrp_insert_avpgrp(void *ctx, DM_AVPGRP **avpgrp, DM_AVPGRP *source) {
	return dm_avpgrp_insert_raw(ctx, avpgrp, (uint8_t*)source + sizeof(DM_AVPGRP), source->info.avps_length);
}

static inline int
dm_get_string_avp(char *dest, size_t dlen, const void *src, size_t slen) { 
	size_t len = slen;

	if (len >= dlen)
		len = dlen - 1;

	memcpy(dest, src, len);
	dest[len] = '\0';

	return len;
}

static inline int8_t
dm_get_int8_avp(const void *src) {
	return *(int8_t *)src;
}

static inline uint8_t
dm_get_uint8_avp(const void *src) {
	return *(uint8_t *)src;
}

static inline int16_t
dm_get_int16_avp(const void *src) {
	return ntohs(*(int16_t *)src);
}

static inline uint16_t
dm_get_uint16_avp(const void *src) {
	return ntohs(*(uint16_t *)src);
}

static inline int32_t
dm_get_int32_avp(const void *src) {
	return ntohl(*(int32_t *)src);
}

static inline uint32_t
dm_get_uint32_avp(const void *src) {
	return ntohl(*(uint32_t *)src);
}

static inline int64_t
dm_get_int64_avp(const void *src) {
	return ntohll(*(int64_t *)src);
}

static inline uint64_t
dm_get_uint64_avp(const void *src) {
	return ntohll(*(uint64_t *)src);
}

static inline time_t
dm_get_time_avp(const void *src) {
	uint32_t val = dm_get_uint32_avp(src);

	return val - TIME_SINCE_EPOCH;
}

static inline struct timeval
dm_get_timeval_avp(const void *src) {
	DM_TIMEVAL	*input = (void*)src;
	struct timeval	ret;

	ret.tv_sec = ntohl((uint32_t)input->tv_sec);
	ret.tv_usec = ntohl((uint32_t)input->tv_usec);

	return ret;
}

#endif
