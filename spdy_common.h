#include <sys/types.h>

#define VERSION(v)  (__bswap_16(v->version) & 0x7FFF)
#define TYPE(v)     (__bswap_16(v->type))
#define LENGTH(v)   (__bswap_32(v->length << 8) & 0x00FFFFFF)
#define STREAMID(v) (__bswap_32(v->streamid) & 0x7FFFFFFF)

typedef struct {
  union {
    struct {
      u_int8_t unused0:8;
      u_int8_t control:1;
      u_int8_t unused1:7;
    } __attribute__((__packed__));
    u_int16_t version;
  };
  u_int16_t type;
  u_int8_t  flag;
  u_int32_t length:24;
  unsigned char data[];
} __attribute__((__packed__)) SPDY_CONTROL_FRAME;

typedef struct {
  union {
    struct {
      u_int8_t unused0:7;
      u_int8_t control:1;
      u_int8_t unused1:8;
      u_int8_t unused2:8;
      u_int8_t unused3:8;
    } __attribute__((__packed__));
    u_int32_t streamid;
  };
  unsigned int  flag:8;
  unsigned int  length:24;
  unsigned char data[];
} __attribute__((__packed__)) SPDY_DATA_FRAME;
