/**
 * bytebuffer.h
 * Copyright (C) 2017 Alexander Koch
 */

#ifndef bytebuffer_h
#define bytebuffer_h

#include <stdint.h>
#include <core/mem.h>

typedef struct {
    uint8_t* data;
    int count;
    int capacity;
} bytebuffer_t;

void bytebuffer_init(bytebuffer_t* buffer);
void bytebuffer_clear(bytebuffer_t* buffer);
void bytebuffer_fill(bytebuffer_t* buffer, uint8_t data, int count);
void bytebuffer_write(bytebuffer_t* buffer, uint8_t data);

#endif
