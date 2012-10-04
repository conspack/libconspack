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

#ifndef CONSPACK_H
#define CONSPACK_H

#include <stdint.h>
#include <stdlib.h>

 /* Headers */

#define CPK_BOOL                  0x00
#define CPK_BOOL_MASK             0xFE

#define CPK_NIL                   0x00
#define CPK_FALSE                 0x00
#define CPK_TRUE                  0x01

#define CPK_NUMBER                0x10
#define CPK_NUMBER_MASK           0xF0

#define CPK_CONTAINER             0x20
#define CPK_CONTAINER_MASK        0xE0

#define CPK_STRING                0x40
#define CPK_STRING_MASK           0xFC

#define CPK_REF                   0x60
#define CPK_REF_MASK              0xFC
#define CPK_REF_INLINE_MASK       0xF0

#define CPK_REMOTE_REF            0x64
#define CPK_REMOTE_REF_MASK       0xFF

#define CPK_TAG                   0xE0
#define CPK_TAG_MASK              0xFC
#define CPK_TAG_INLINE_MASK       0xF0

#define CPK_CONS                  0x80
#define CPK_CONS_MASK             0xFF

#define CPK_PACKAGE               0x81
#define CPK_PACKAGE_MASK          0xFF

#define CPK_SYMBOL                0x82
#define CPK_SYMBOL_MASK           0xFE

#define CPK_INDEX                 0xA0
#define CPK_INDEX_MASK            0xE0

#define CPK_ERROR                 -1

#define CPK_SIZE_8        0x00
#define CPK_SIZE_16       0x01
#define CPK_SIZE_32       0x02
#define CPK_SIZE_MASK     0x03

#define CPK_CONTAINER_VECTOR    0x00
#define CPK_CONTAINER_LIST      0x08
#define CPK_CONTAINER_MAP       0x10
#define CPK_CONTAINER_TMAP      0x18
#define CPK_CONTAINER_TYPE_MASK 0x18
#define CPK_CONTAINER_FIXED     0x04

#define CPK_REFTAG_INLINE 0x10
#define CPK_SYMBOL_KEYWORD 0x01

#define CPK_INT8 0x0
#define CPK_INT16 0x1
#define CPK_INT32 0x2
#define CPK_INT64 0x3
#define CPK_UINT8 0x4
#define CPK_UINT16 0x5
#define CPK_UINT32 0x6
#define CPK_UINT64 0x7
#define CPK_SINGLE_FLOAT 0x8
#define CPK_DOUBLE_FLOAT 0x9
#define CPK_INT128 0xA
#define CPK_UINT128 0xB
#define CPK_COMPLEX 0xC
#define CPK_RATIONAL 0xF

#define CPK_NUMBER_TYPE(h) ((h) & 0xF)

/* Checking */

#define CPK_IS_BOOL(h) (((h) & CPK_BOOL_MASK) == CPK_BOOL)
#define CPK_IS_NUMBER(h) (((h) & CPK_NUMBER_MASK) == CPK_NUMBER)
#define CPK_IS_CONTAINER(h) (((h) & CPK_CONTAINER_MASK) == CPK_CONTAINER)
#define CPK_IS_STRING(h) (((h) & CPK_STRING_MASK) == CPK_STRING)
#define CPK_IS_REF(h) ((((h) & CPK_REF_MASK) == CPK_REF) || \
                       (((h) & CPK_REF_INLINE_MASK) == (CPK_REF | CPK_REFTAG_INLINE)))
#define CPK_IS_REMOTE_REF(h) (((h) & CPK_REMOTE_REF) == CPK_REMOTE_REF)
#define CPK_IS_TAG(h) ((((h) & CPK_TAG_MASK) == CPK_TAG) || \
                       (((h) & CPK_TAG_INLINE_MASK) == (CPK_TAG | CPK_REFTAG_INLINE)))
#define CPK_IS_CONS(h) (((h) & CPK_CONS_MASK) == CPK_CONS)
#define CPK_IS_PACKAGE(h) (((h) & CPK_PACKAGE_MASK) == CPK_PACKAGE)
#define CPK_IS_SYMBOL(h) (((h) & CPK_SYMBOL_MASK) == CPK_SYMBOL)
#define CPK_IS_KEYWORD(h) (CPK_IS_SYMBOL(h) && ((h) & 1))
#define CPK_IS_INDEX(h) (((h) & CPK_INDEX_MASK) == CPK_INDEX)

#define CPK_IS_ERROR(h) ((h) == CPK_ERROR)

 /* Encoding */

#define CPK_DEFAULT_BUFFER 16

typedef struct _cpk_output {
    size_t buffer_size;
    size_t buffer_used;
    unsigned char *buffer;

    int fd;
} cpk_output_t;

void cpk_output_init(cpk_output_t *out);
void cpk_output_init_fd(cpk_output_t *out, int fd);
void cpk_output_fini(cpk_output_t *out);
void cpk_output_clear(cpk_output_t *out);

int cpk_write8(cpk_output_t *out, uint8_t val);
int cpk_write16(cpk_output_t *out, uint16_t val);
int cpk_write32(cpk_output_t *out, uint32_t val);
int cpk_write64(cpk_output_t *out, uint64_t val);

int cpk_write_single(cpk_output_t *out, float val);
int cpk_write_double(cpk_output_t *out, double val);

int cpk_write_bytes(cpk_output_t *out, const uint8_t *val, size_t len);
int cpk_write_string(cpk_output_t *out, const char *str);
int cpk_snprintf(cpk_output_t *out, size_t size, const char *fmt, ...);

int cpk_print(cpk_output_t *out);

void cpk_encode_container(cpk_output_t *out, uint8_t type,
                          uint32_t size, uint8_t fixed_header);

void cpk_encode_string(cpk_output_t *out, const char *str);

void cpk_encode_ref(cpk_output_t *out, uint8_t type, uint32_t val);

 /* Decoding */

typedef struct _cpk_bool {
    int16_t header;
    uint8_t val;
} cpk_bool_t;

typedef struct _cpk_number {
    int16_t header;

    union {
        uint8_t uint8;
        uint16_t uint16;
        uint32_t uint32;
        uint64_t uint64;

        int8_t int8;
        int16_t int16;
        int32_t int32;
        int64_t int64;

        uint8_t int128_bytes[16];
        uint8_t uint128_bytes[16];

        float single_float;
        double double_float;
    } val;
} cpk_number_t;

typedef struct _cpk_rational {
    int16_t header;
    union _cpk_object *n, *d;
} cpk_rational_t;

typedef struct _cpk_complex {
    int16_t header;
    union _cpk_object *r, *i;
} cpk_complex_t;

typedef struct _cpk_container {
    int16_t header;
    uint32_t size;
    uint8_t fixed_header;
    union _cpk_object **obj;
} cpk_container_t;

typedef struct _cpk_string {
    int16_t header;
    uint32_t size;
    uint8_t *data;
} cpk_string_t;

typedef struct _cpk_ref {
    int16_t header;
    uint32_t val;
} cpk_ref_t, cpk_tag_t, cpk_index_t;

typedef struct _cpk_remote_ref {
    int16_t header;
    union _cpk_object *val;
} cpk_remote_ref_t;

typedef struct _cpk_cons {
    int16_t header;
    union _cpk_object *car, *cdr;
} cpk_cons_t;

typedef struct _cpk_package {
    int16_t header;
    union _cpk_object *name;
} cpk_package_t;

typedef struct _cpk_symbol {
    int16_t header;
    union _cpk_object *package, *name;
} cpk_symbol_t;

typedef struct _cpk_error {
    int16_t header;
    uint32_t code;
    char *reason;
    uint8_t value;
    size_t pos;
} cpk_error_t;

#define CPK_ERR_EOF 0x00
#define CPK_ERR_BAD_HEADER 0x01
#define CPK_ERR_BAD_SIZE 0x02
#define CPK_ERR_BAD_TYPE 0x03

extern const char *CPK_ERR_EOF_MSG;
extern const char *CPK_ERR_BAD_HEADER_MSG;
extern const char *CPK_ERR_BAD_SIZE_MSG;
extern const char *CPK_ERR_BAD_TYPE_MSG;

typedef union _cpk_object {
    int16_t header;

    cpk_bool_t bool;
    cpk_number_t number;
    cpk_rational_t rational;
    cpk_complex_t complex;
    cpk_container_t container;
    cpk_string_t string;
    cpk_ref_t ref, tag, index;
    cpk_remote_ref_t rref;
    cpk_cons_t cons;
    cpk_package_t package;
    cpk_symbol_t symbol;
    cpk_error_t error;
} cpk_object_t;

typedef struct _cpk_input {
    size_t buffer_size;
    size_t buffer_read;
    uint8_t *buffer;

    int fd;
} cpk_input_t;

void cpk_input_init(cpk_input_t *in, uint8_t *data, size_t len);
void cpk_input_init_fd(cpk_input_t *in, int fd);

int cpk_read8(cpk_input_t *in, uint8_t *dest);
int cpk_read16(cpk_input_t *in, uint16_t *dest);
int cpk_read32(cpk_input_t *in, uint32_t *dest);
int cpk_read64(cpk_input_t *in, uint64_t *dest);
int cpk_read_bytes(cpk_input_t *in, uint8_t *dest, size_t len);

uint8_t cpk_decode_header(uint8_t header);
void cpk_decode(cpk_input_t *in, cpk_object_t *obj, int skip_header);
cpk_object_t* cpk_decode_r(cpk_input_t *in);
cpk_object_t* cpk_decode_rh(cpk_input_t *in, uint8_t header);

void cpk_free(cpk_object_t *obj);
void cpk_free_r(cpk_object_t *obj);

 /* Explain */

extern const char *CPK_BOOL_STR;
extern const char *CPK_NIL_STR;
extern const char *CPK_TRUE_STR;
extern const char *CPK_NUMBER_STR;
extern const char *CPK_STRING_STR;
extern const char *CPK_REF_STR;
extern const char *CPK_REMOTE_REF_STR;
extern const char *CPK_TAG_STR;
extern const char *CPK_CONS_STR;
extern const char *CPK_PACKAGE_STR;
extern const char *CPK_SYMBOL_STR;
extern const char *CPK_INDEX_STR;

extern const char *CPK_VECTOR_STR;
extern const char *CPK_LIST_STR;
extern const char *CPK_MAP_STR;
extern const char *CPK_TMAP_STR;

extern const char *CPK_INLINE_STR;
extern const char *CPK_FIXED_STR;
extern const char *CPK_KEYWORD_STR;

extern const char *CPK_INT_STR;
extern const char *CPK_UINT_STR;
extern const char *CPK_SINGLE_FLOAT_STR;
extern const char *CPK_DOUBLE_FLOAT_STR;
extern const char *CPK_COMPLEX_STR;
extern const char *CPK_RATIONAL_STR;

void cpk_explain_object(cpk_output_t *out, cpk_object_t *obj);

#endif /* CONSPACK_H */
