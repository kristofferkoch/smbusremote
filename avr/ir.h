#ifndef _IR_H
#define _IR_H
#include <inttypes.h>
#define FIRST_MIN (20000)
#define FIRST_MAX (25000)
#define T (3475)
#define KEY_TIMEOUT (26)
#define KEY_DOWN (0x0100)
#define KEY_UP   (0x0200)

typedef struct ir_event {
  uint8_t code;
  uint8_t repititions;
} ir_event;

void ir_init(void);
int8_t ir_getcode(ir_event *evt);

#endif
