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

#include <stdio.h>
#include <memory.h>
#include <string.h>

const char *CPK_ERR_EOF_MSG = "End of input";
const char *CPK_ERR_BAD_HEADER_MSG = "Bad header value";
const char *CPK_ERR_BAD_SIZE_MSG = "Bad size type";
const char *CPK_ERR_BAD_TYPE_MSG = "Bad type";

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
        *dest = net16(*(uint16_t*)(in->buffer + in->buffer_read));
        in->buffer_read += 2;
    }

    return 2;
}

int cpk_read32(cpk_input_t *in, uint32_t *dest) {
    if(in->fd >= 0)
        return read(in->fd, dest, 4);
    else {
        if(!cpk_input_has(in, 4)) return -1;
        *dest = net32(*(uint32_t*)(in->buffer + in->buffer_read));
        in->buffer_read += 4;
    }

    return 4;
}

int cpk_read64(cpk_input_t *in, uint64_t *dest) {
    if(in->fd >= 0)
        return read(in->fd, dest, 8);
    else {
        if(!cpk_input_has(in, 8)) return -1;
        *dest = net64(*(uint64_t*)(in->buffer + in->buffer_read));
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

void cpk_err(cpk_object_t *obj, uint32_t code, const char *reason,
             uint8_t value, size_t pos) {
    obj->header = CPK_ERROR;
    obj->error.code = code;
    obj->error.reason = (char*)reason;
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

#define READN(len,src,dest,err) \
{   cpk_input_t *_in = (src); \
    if(cpk_read_bytes(_in,(dest),(len)) < 0) { \
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
    cpk_decode(__in, dest, 0);         \
    if(CPK_IS_ERROR(dest->header)) { \
        *__err = *dest; \
        free(dest); \
        dest = NULL; \
    } \
 \
    if(!test(dest->header)) { \
        cpk_err(__err, CPK_ERR_BAD_TYPE, CPK_ERR_BAD_TYPE_MSG, \
                dest->header, __in->buffer_read); \
        free(dest); \
        dest = NULL; \
    } \
}

uint8_t cpk_decode_header(uint8_t header) {
    uint8_t tmp = header & 0xF0;
    uint8_t ref = header & 0xE0;

    if(header == 0x64)
        return header;
    else if(ref == CPK_REF || ref == CPK_TAG || ref == CPK_INDEX)
        return ref;
    else if((header & CPK_SYMBOL_MASK) == CPK_SYMBOL)
        return CPK_SYMBOL;
    else if(tmp == 0x80)
        return header;

    return tmp;
}

void cpk_decode_number(cpk_input_t *in, cpk_object_t *obj, uint8_t h) {
    cpk_object_t *a = NULL, *b = NULL;
    
    switch(CPK_NUMBER_TYPE(h)) {
        case CPK_INT8:
        case CPK_UINT8:
            READ(8, in, &obj->number.val.uint8, obj);
            break;

        case CPK_INT16:
        case CPK_UINT16:
            READ(16, in, &obj->number.val.uint16, obj);
            break;

        case CPK_INT32:
        case CPK_UINT32:
            READ(32, in, &obj->number.val.uint32, obj);
            break;

        case CPK_INT64:
        case CPK_UINT64:
            READ(64, in, &obj->number.val.uint64, obj);
            break;

        case CPK_SINGLE_FLOAT:
            READ(32, in, &obj->number.val.uint32, obj);
            break;

        case CPK_DOUBLE_FLOAT:
            READ(64, in, &obj->number.val.uint64, obj);
            break;

        case CPK_COMPLEX:
        case CPK_RATIONAL:
            DECODE_TEST(in, a, CPK_IS_NUMBER, obj);
            if(!a) return;
            
            DECODE_TEST(in, b, CPK_IS_NUMBER, obj);
            if(!b) { cpk_free(b); free(b); return; }

            obj->complex.r = a;
            obj->complex.i = b;
            break;

        default:
            cpk_err(obj, CPK_ERR_BAD_HEADER, CPK_ERR_BAD_HEADER_MSG,
                    h, in->buffer_read);
    }            
}

uint32_t
cpk_decode_size(cpk_input_t *in, uint8_t header, cpk_object_t *err) {
    uint8_t i8;
    uint16_t i16;
    uint32_t i32;

    switch(header & CPK_SIZE_MASK) {
        case CPK_SIZE_8:  READ(8, in, &i8, err); return i8;
        case CPK_SIZE_16: READ(16, in, &i16, err); return i16;
        case CPK_SIZE_32: READ(32, in, &i32, err); return i32;
    }
}

void cpk_decode(cpk_input_t *in, cpk_object_t *obj, int skip_header) {
    uint8_t header;

    if(!skip_header) {
        READ(8, in, &header, obj);
        obj->header = (unsigned short)header;
    } else {
        header = obj->header;
    }

    switch(cpk_decode_header(obj->header)) {
        case CPK_BOOL:
            READ(8, in, &obj->bool.val, obj);
            break;

        case CPK_NUMBER:
            cpk_decode_number(in, obj, header);
            break;
            
        case CPK_CONTAINER:
            obj->container.size = cpk_decode_size(in, header, obj);
            obj->container.obj  = NULL;
            if(header & CPK_CONTAINER_FIXED)
                READ(8, in, &obj->container.fixed_header, obj);
            if(header & CPK_CONTAINER_MAP)
                obj->container.size *= 2;
            break;
            
        case CPK_STRING:
            obj->string.size = cpk_decode_size(in, header, obj);
            obj->string.data = malloc(obj->string.size+1);
            READN(obj->string.size, in, obj->string.data, obj);
            obj->string.data[obj->string.size] = 0;
            break;

        case CPK_REF:
        case CPK_TAG:
        case CPK_INDEX:
            if(header & CPK_REFTAG_INLINE)
                obj->ref.val = header & 0xF;
            else
                obj->ref.val = cpk_decode_size(in, header, obj);
            
            break;

        case CPK_REMOTE_REF:
            obj->rref.val = NULL;
            break;
            
        case CPK_CONS:
            obj->cons.car = NULL;
            obj->cons.cdr = NULL;
            break;

        case CPK_PACKAGE:
            obj->package.name = NULL;
            break;

        case CPK_SYMBOL:
            obj->symbol.name = NULL;
            obj->symbol.package = NULL;
            break;

        default:
            cpk_err(obj, CPK_ERR_BAD_HEADER, CPK_ERR_BAD_HEADER_MSG,
                    header, in->buffer_read);
    }
}

void cpk_free(cpk_object_t *obj) {
    if(CPK_IS_NUMBER(obj->header)) {
        if(CPK_NUMBER_TYPE(obj->header) == CPK_COMPLEX ||
           CPK_NUMBER_TYPE(obj->header) == CPK_RATIONAL) {
            cpk_free(obj->complex.r);
            cpk_free(obj->complex.i);
            
            free(obj->complex.r);
            free(obj->complex.i);
        }
    } else if(CPK_IS_STRING(obj->header)) {
        free(obj->string.data);
    } else if(CPK_IS_REMOTE_REF(obj->header)) {
        cpk_free(obj->rref.val);
        free(obj->rref.val);
    } else if(CPK_IS_CONTAINER(obj->header) && obj->container.obj) {
        free(obj->container.obj);
    }
}

cpk_object_t* cpk_decode_r(cpk_input_t *in) {
    return cpk_decode_rh(in, 0);
}

cpk_object_t* cpk_decode_rh(cpk_input_t *in, uint8_t header) {
    cpk_object_t *obj = calloc(1, sizeof(cpk_object_t)),
                 *tmp = NULL;
    uint32_t i = 0;

    if(header)
        obj->header = header;

    cpk_decode(in, obj, header != 0);
    if(CPK_IS_ERROR(obj->header))
        goto error;

    switch(cpk_decode_header(obj->header)) {
        case CPK_REMOTE_REF:
            tmp = cpk_decode_rh(in, 0);
            if(CPK_IS_ERROR(tmp->header))
                goto error;
            
            obj->rref.val = tmp;
            break;

        case CPK_CONS:
            tmp = cpk_decode_rh(in, 0);
            if(CPK_IS_ERROR(tmp->header))
                goto error;

            obj->cons.car = tmp;

            tmp = cpk_decode_rh(in, 0);
            if(CPK_IS_ERROR(tmp->header))
                goto error;

            obj->cons.cdr = tmp;
            break;

        case CPK_CONTAINER:
            obj->container.obj = calloc(obj->container.size,
                                        sizeof(cpk_object_t*));
            for(i = 0; i < obj->container.size; i++) {
                tmp = cpk_decode_rh(in, obj->container.fixed_header);
                if(CPK_IS_ERROR(tmp->header))
                    goto error;
                obj->container.obj[i] = tmp;
            }

            break;
    }

 end:
    return obj;

 error:
    if(CPK_IS_ERROR(obj->header))
        return obj;

    cpk_free_r(obj);
    return tmp;
}

void cpk_free_r(cpk_object_t *obj) {
    cpk_object_t *tmp = NULL;
    uint32_t i = 0;
    
    if(!obj) return;

    switch(cpk_decode_header(obj->header)) {
        case CPK_REMOTE_REF:
            cpk_free_r(obj->rref.val);
            break;

        case CPK_CONS:
            cpk_free_r(obj->cons.car);
            cpk_free_r(obj->cons.cdr);
            break;

        case CPK_CONTAINER:
            for(i = 0; i < obj->container.size; i++) {
                if(obj->container.obj[i])
                    cpk_free_r(obj->container.obj[i]);
                else
                    break;
            }

            free(obj->container.obj);
            break;
    }

    free(obj);
}
