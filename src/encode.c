/*
 * libconspack
 * Copyright (C) 2012  Ryan Pavlik
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public
 * License (LGPL) version 2.1 which accompanies this distribution, and
 * is available at http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "config.h"
#include "conspack/conspack.h"

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

void cpk_output_init(cpk_output_t *out) {
    out->buffer_size = CPK_DEFAULT_BUFFER;
    out->buffer_used = 0;
    out->buffer      = malloc(CPK_DEFAULT_BUFFER);
    out->fd          = -1;
}

void cpk_output_init_fd(cpk_output_t *out, int fd) {
    out->fd          = fd;
    out->buffer_size = 0;
    out->buffer_used = 0;
    out->buffer      = NULL;
}

void cpk_output_fini(cpk_output_t *out) {
    if(out->buffer) {
        free(out->buffer);
        out->buffer_size = 0;
        out->buffer_used = 0;
    }
}

void cpk_output_clear(cpk_output_t *out) {
    out->buffer_used = 0;
}

void cpk_ensure_buffer(cpk_output_t *out, size_t bytes_needed) {
    if((out->buffer_used + bytes_needed) <= out->buffer_size)
        return;

    out->buffer_size = 2 * out->buffer_size;
    out->buffer      = realloc(out->buffer, out->buffer_size);
}

int cpk_write8(cpk_output_t *out, uint8_t val) {
    if(out->fd >= 0)
        return write(out->fd, &val, 1);
    else {
        cpk_ensure_buffer(out, 1);
        out->buffer[out->buffer_used] = val;
        out->buffer_used++;
        return 1;
    }
}

int cpk_write16(cpk_output_t *out, uint16_t val) {
    val = net16(val);

    if(out->fd >= 0)
        return write(out->fd, &val, 2);
    else {
        cpk_ensure_buffer(out, 2);
        *(uint16_t*)(out->buffer + out->buffer_used) = val;
        out->buffer_used += 2;
        return 2;
    }
}

int cpk_write32(cpk_output_t *out, uint32_t val) {
    val = net32(val);

    if(out->fd >= 0)
        return write(out->fd, &val, 4);
    else {
        cpk_ensure_buffer(out, 4);
        *(uint32_t*)(out->buffer + out->buffer_used) = val;
        out->buffer_used += 4;
        return 4;
    }
}

int cpk_write64(cpk_output_t *out, uint64_t val) {
    val = net64(val);

    if(out->fd >= 0)
        return write(out->fd, &val, 8);
    else {
        cpk_ensure_buffer(out, 8);
        *(uint64_t*)(out->buffer + out->buffer_used) = val;
        out->buffer_used += 8;
        return 8;
    }
}

int cpk_write_single(cpk_output_t *out, float val) {
    uint32_t *ptr = (uint32_t*)&val;
    return cpk_write32(out, *ptr);
}

int cpk_write_double(cpk_output_t *out, double val) {
    uint64_t *ptr = (uint64_t*)&val;
    return cpk_write64(out, *ptr);
}

int cpk_write_bytes(cpk_output_t *out, const uint8_t *val, size_t len) {
    if(out->fd >= 0)
        return write(out->fd, &val, len);
    else {
        cpk_ensure_buffer(out, len);
        memcpy(out->buffer + out->buffer_used, val, len);
        out->buffer_used += len;
        return len;
    }
}

int cpk_write_string(cpk_output_t *out, const char *str) {
   return cpk_write_bytes(out, (uint8_t*)str, strlen(str));
}

int cpk_print(cpk_output_t *out) {
    return printf("%*s\n", (int)out->buffer_used, out->buffer);
}

int cpk_snprintf(cpk_output_t *out, size_t size, const char *fmt, ...) {
    va_list ap;
    int count = 0;
    va_start(ap, fmt);
    
    cpk_ensure_buffer(out, size);
    count = vsnprintf(out->buffer + out->buffer_used, size, fmt, ap);
    out->buffer_used += count; /* This excludes \0 */
    
    va_end(ap);

    return count;
}
    

void cpk_encode_size_header(cpk_output_t *out, uint8_t header,
                            uint32_t size) {
    int size_type = 0;

    if(size & 0xFFFF0000)      size_type = CPK_SIZE_32;
    else if(size & 0x0000FF00) size_type = CPK_SIZE_16;

    cpk_write8(out, header | size_type);

    switch(size_type) {
        case CPK_SIZE_8:  cpk_write8(out, (uint8_t)size);   break;
        case CPK_SIZE_16: cpk_write16(out, (uint16_t)size); break;
        case CPK_SIZE_32: cpk_write32(out, (uint32_t)size);
    }
}

void cpk_encode_container(cpk_output_t *out, uint8_t type,
                          uint32_t size, uint8_t fixed_header) {
    uint8_t header = CPK_CONTAINER;

    if(fixed_header) header |= CPK_CONTAINER_FIXED;

    cpk_encode_size_header(out, header, size);

    if(fixed_header) cpk_write8(out, fixed_header);
}

void cpk_encode_string(cpk_output_t *out, const char *str) {
    size_t len = strlen(str);

    cpk_encode_size_header(out, CPK_STRING, len);
    cpk_write_bytes(out, str, len);
}

void cpk_encode_ref(cpk_output_t *out, uint8_t type, uint32_t val) {
    if(val < 16)
        cpk_write8(out, type | CPK_REFTAG_INLINE | val);
    else
        cpk_encode_size_header(out, type, val);
}
