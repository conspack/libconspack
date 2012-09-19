/*
 * libconspack
 * Copyright (C) 2012  Ryan Pavlik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "config.h"
#include "conspack/conspack.h"

#include <memory.h>
#include <string.h>

void cpk_input_init(cpk_input_t *in, uint8_t *data, size_t len) {
    in->buffer = data;
    in->buffer_read = 0;
    in->buffer_size = len;

    in->fd = -1;
}

void cpk_input_init_fd(cpk_input_t *in, int fd) {
    in->buffer = NULL;
    in->buffer_read = 0;
    in->buffer_size = 0;

    in->fd = fd;
}

int cpk_input_has(cpk_input_t *in, size_t bytes) {
    return (in->buffer_size - in->buffer_read) >= bytes;
}

int cpk_read8(cpk_input_t *in, uint8_t *dest) {
    if(in->fd >= 0)
        return read(in->fd, dest, 1);
    else {
        if(!cpk_input_has(in, 1)) return -1;
        *dest = in->buffer[in->buffer_read];
        in->buffer_read++;
    }

    return 1;
}

int cpk_read16(cpk_input_t *in, uint16_t *dest) {
    if(in->fd >= 0)
        return read(in->fd, dest, 2);
    else {
        if(!cpk_input_has(in, 2)) return -1;
        *dest = *(uint16_t*)(in->buffer + in->buffer_read);
        in->buffer_read += 2;
    }

    return 2;
}

int cpk_read32(cpk_input_t *in, uint32_t *dest) {
    if(in->fd >= 0)
        return read(in->fd, dest, 4);
    else {
        if(!cpk_input_has(in, 4)) return -1;
        *dest = *(uint32_t*)(in->buffer + in->buffer_read);
        in->buffer_read += 4;
    }

    return 4;
}

int cpk_read64(cpk_input_t *in, uint64_t *dest) {
    if(in->fd >= 0)
        return read(in->fd, dest, 8);
    else {
        if(!cpk_input_has(in, 8)) return -1;
        *dest = *(uint64_t*)(in->buffer + in->buffer_read);
        in->buffer_read += 8;
    }

    return 8;
}

int cpk_read_bytes(cpk_input_t *in, uint8_t *dest, size_t len) {
    if(in->fd >= 0)
        read(in->fd, dest, len);
    else {
        if(!cpk_input_has(in, len)) return -1;
        memcpy(dest, (in->buffer + in->buffer_read), len);
        in->buffer_read += len;
    }

    return len;
}

void cpk_err(cpk_object_t *obj, uint32_t code, char *reason,
             uint8_t value, size_t pos) {
    obj->header = CPK_ERROR;
    obj->error.code = code;
    obj->error.reason = reason;
    obj->error.value = value;
    obj->error.pos = pos;
}

#define READ(n,src,dest,err) \
{   cpk_input_t *_in = (src); \
    if(cpk_read##n(_in,(dest)) < 0) { \
        cpk_err((err), CPK_ERR_EOF, CPK_ERR_EOF_MSG, 0, _in->buffer_read); \
        return; \
    } \
}

#define DECODE_TEST(in,dest,test,err) \
{   cpk_input_t *__in = (in); \
    cpk_object_t *__err = (err); \
 \
    dest = malloc(sizeof(cpk_object_t)); \
 \
    cpk_decode(__in, dest); \
    if(CPK_IS_ERROR(dest->header)) { \
        *__err = *dest; \
        free(dest); \
        return; \
    } \
 \
    if(!test(dest->header)) { \
        cpk_err(__err, CPK_ERR_BAD_TYPE, CPK_ERR_BAD_TYPE_MSG, \
                dest->header, __in->buffer_read); \
        free(dest); \
        return; \
    } \
}

void cpk_decode_number(cpk_input_t *in, cpk_object_t *obj, uint8_t h) {
    cpk_object_t *a = NULL, *b = NULL;
    
    switch(CPK_NUMBER_TYPE(h)) {
        case CPK_INT8:
        case CPK_UINT8:
            READ(8, in, &obj->number.uint8, obj);
            break;

        case CPK_INT16:
        case CPK_UINT16:
            READ(16, in, &obj->number.uint16, obj);
            break;

        case CPK_INT32:
        case CPK_UINT32:
            READ(32, in, &obj->number.uint32, obj);
            break;

        case CPK_INT64:
        case CPK_UINT64:
            READ(64, in, &obj->number.uint64, obj);
            break;

        case CPK_SINGLE_FLOAT:
            READ(32, in, &obj->number.uint32, obj);
            break;

        case CPK_DOUBLE_FLOAT:
            READ(64, in, &obj->number.uint64, obj);
            break;

        case CPK_COMPLEX:
        case CPK_RATIONAL:
            DECODE_TEST(in, a, CPK_IS_NUMBER, obj);
            DECODE_TEST(in, b, CPK_IS_NUMBER, obj);

            obj->complex.r = a;
            obj->complex.i = b;
            break;

        default:
            cpk_err(obj, CPK_ERR_BAD_HEADER, CPK_ERR_BAD_HEADER_MSG,
                    h, in->buffer_read);
    }            
}

void cpk_decode(cpk_input_t *in, cpk_object_t *obj) {
    uint8_t header;

    READ(8, in, &header, obj);

    if(CPK_IS_BOOL(header)) {
        READ(8, in, &obj->bool.val, obj);
        return;
    } else if(CPK_IS_NUMBER(header)) {
        cpk_decode_number(in, obj, header);
        return;
    }
}
