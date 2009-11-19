#include <stdlib.h>
#include "i2c-dev.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

typedef struct data {
  int file;
  unsigned char addr;
} data_t;

void *init(char *fn, unsigned char addr) {
  data_t *d = malloc(sizeof(data_t));
  if(d==NULL) goto error0;

  d->file = open(fn, O_RDWR);
  if (d->file<0) {
    printf("Error opening: %s (%d)\n",strerror(errno),errno);
    goto error1;
  }
  d->addr = addr & 0x7F;
  if (ioctl(d->file, I2C_SLAVE, addr) < 0) goto error2;

  return (void *)d;
 error2:
  close(d->file);
 error1:
  free(d);
 error0:
  return NULL;
}

int getcode(void *data, unsigned char cmd) {
  data_t *d = data;
  return i2c_smbus_read_word_data(d->file, cmd); 
}

int setled(void *data, unsigned char cmd, int setting) {
  data_t *d = data;
  return i2c_smbus_write_word_data(d->file, cmd, setting & 0xFFFF);
}
void deinit(void *data) {
  data_t *d = data;
  close(d->file);
  free(data);
}
