#ifndef __AOA_SKELETON_H
#define __AOA_SKELETON_H

#include "adk-common.h"

static ssize_t aoa_read(struct file *file, char *buffer, size_t count,loff_t *ppos);
static ssize_t aoa_write(struct file *file, const char *user_buffer,size_t count, loff_t *ppos);

#endif
