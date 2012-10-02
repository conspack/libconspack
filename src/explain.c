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
#include <string.h>
#include <inttypes.h>

const char *CPK_BOOL_STR = ":boolean";
const char *CPK_NIL_STR = "nil";
const char *CPK_TRUE_STR = "t";
const char *CPK_NUMBER_STR = ":number";
const char *CPK_STRING_STR = ":string";
const char *CPK_REF_STR = ":ref";
const char *CPK_REMOTE_REF_STR = ":rref";
const char *CPK_TAG_STR = ":tag";
const char *CPK_CONS_STR = ":cons";
const char *CPK_PACKAGE_STR = ":package";
const char *CPK_SYMBOL_STR = ":symbol";
const char *CPK_INDEX_STR = ":index";

const char *CPK_VECTOR_STR = ":vector";
const char *CPK_LIST_STR = ":list";
const char *CPK_MAP_STR = ":map";
const char *CPK_TMAP_STR = ":tmap";

const char *CPK_INLINE_STR = ":inline";
const char *CPK_FIXED_STR = ":fixed";
const char *CPK_KEYWORD_STR = ":keyword";

const char *CPK_INT_STR = ":int";
const char *CPK_UINT_STR = ":uint";
const char *CPK_SINGLE_FLOAT_STR = ":single-float";
const char *CPK_DOUBLE_FLOAT_STR = ":double-float";
const char *CPK_COMPLEX_STR = ":complex";
const char *CPK_RATIONAL_STR = ":rational";

static void explain_object_r(cpk_output_t *out, cpk_object_t *obj);

static void explain_bool(cpk_output_t *out, cpk_object_t *obj) {
    cpk_write_string(out, CPK_BOOL_STR);
    cpk_write_string(out, " ");

    if(obj->header == CPK_NIL)
        cpk_write_string(out, CPK_NIL_STR);
    else
        cpk_write_string(out, CPK_TRUE_STR);
}

static void explain_int(cpk_output_t *out, cpk_object_t *obj) {
    int size = obj->header ^ CPK_NUMBER;

    if(size < CPK_UINT8 || size == CPK_INT128)
        cpk_write_string(out, CPK_INT_STR);
    else
        cpk_write_string(out, CPK_UINT_STR);

    switch(size) {
        case CPK_INT8:
            cpk_snprintf(out, 20, "8 %" PRId32, (int32_t)obj->number.val.int8);
            break;
            
        case CPK_UINT8:
            cpk_snprintf(out, 20, "8 %" PRIu32, (uint32_t)obj->number.val.uint8);
            break;
            
        case CPK_INT16:
            cpk_snprintf(out, 20, "16 %" PRId32, (int32_t)obj->number.val.int16);
            break;

        case CPK_UINT16:
            cpk_snprintf(out, 20, "16 %" PRIu32, (uint32_t)obj->number.val.uint16);
            break;

        case CPK_INT32:
            cpk_snprintf(out, 20, "32 %" PRId32, (int32_t)obj->number.val.int32);
            break;

        case CPK_UINT32:
            cpk_snprintf(out, 20, "32 %" PRIu32, (uint32_t)obj->number.val.uint32);
            break;

        case CPK_INT64:
            cpk_snprintf(out, 50, "64 %" PRId32, (int64_t)obj->number.val.int64);
            break;

        case CPK_UINT64:
            cpk_snprintf(out, 50, "64 %" PRIu32, (uint64_t)obj->number.val.uint64);
            break;


        case CPK_INT128:
        case CPK_UINT128:
            cpk_write_string(out, "128");
            break;

        default:
            cpk_write_string(out, "??");
    }
}

static void explain_number(cpk_output_t *out, cpk_object_t *obj) {
    unsigned char numtype = CPK_NUMBER_TYPE(obj->header);
            
    cpk_write_string(out, CPK_NUMBER_STR);
    cpk_write_string(out, " ");

    if(numtype < CPK_SINGLE_FLOAT ||
       numtype == CPK_INT128 ||
       numtype == CPK_UINT128)
        explain_int(out, obj);
    else if(numtype == CPK_SINGLE_FLOAT) {
        cpk_write_string(out, CPK_SINGLE_FLOAT_STR);
        cpk_snprintf(out, 10, " %.7f", obj->number.val.single_float);
    } else if(numtype == CPK_DOUBLE_FLOAT) {
        cpk_write_string(out, CPK_DOUBLE_FLOAT_STR);
        cpk_snprintf(out, 20, " %.16g", obj->number.val.double_float);
    } else if(numtype == CPK_RATIONAL) {
        cpk_write_string(out, CPK_RATIONAL_STR);
        cpk_write_string(out, " ");
        explain_object_r(out, obj->rational.n);
        cpk_write_string(out, " ");
        explain_object_r(out, obj->rational.d);
    } else if(numtype = CPK_COMPLEX) {
        cpk_write_string(out, CPK_COMPLEX_STR);
        cpk_write_string(out, " ");
        explain_object_r(out, obj->complex.r);
        cpk_write_string(out, " ");
        explain_object_r(out, obj->complex.i);
    }    
}

static void explain_container(cpk_output_t *out, cpk_object_t *obj) {
    int i = 0;
    
    switch(obj->header & CPK_CONTAINER_TYPE_MASK) {
        case CPK_CONTAINER_VECTOR:
            cpk_write_string(out, CPK_VECTOR_STR);
            break;

        case CPK_CONTAINER_LIST:
            cpk_write_string(out, CPK_LIST_STR);
            break;

        case CPK_CONTAINER_MAP:
            cpk_write_string(out, CPK_MAP_STR);
            break;

        case CPK_CONTAINER_TMAP:
            cpk_write_string(out, CPK_TMAP_STR);
            break;
    }

    for(i = 0; i < obj->container.size; i++) {
        cpk_write_string(out, " ");
        explain_object_r(out, obj->container.obj[i]);
    }
}

static void explain_string(cpk_output_t *out, cpk_object_t *obj) {
    cpk_write_string(out, CPK_STRING_STR);
    cpk_snprintf(out, (int)obj->string.size + 4,
                 " \"%*s\"", (int)obj->string.size, obj->string.data);
}

static void explain_ref(cpk_output_t *out, cpk_object_t *obj) {
    switch(cpk_decode_header(obj->header)) {
        case CPK_REF: cpk_write_string(out, CPK_REF_STR); break;
        case CPK_TAG: cpk_write_string(out, CPK_TAG_STR); break;
        case CPK_INDEX: cpk_write_string(out, CPK_INDEX_STR); break;
    }

    cpk_snprintf(out, 20, " %" PRIu32, obj->ref.val);
}

static void explain_rref(cpk_output_t *out, cpk_object_t *obj) {
    cpk_write_string(out, CPK_REMOTE_REF_STR);
    cpk_write_string(out, " ");

    explain_object_r(out, obj->rref.val);
}

static void explain_cons(cpk_output_t *out, cpk_object_t *obj) {
    cpk_write_string(out, CPK_CONS_STR);
    cpk_write_string(out, " ");
    explain_object_r(out, obj->cons.car);
    cpk_write_string(out, " ");
    explain_object_r(out, obj->cons.cdr);    
}

static void explain_package(cpk_output_t *out, cpk_object_t *obj) {
    cpk_write_string(out, CPK_PACKAGE_STR);
    cpk_write_string(out, " ");
    explain_object_r(out, obj->package.name);
}

static void explain_symbol(cpk_output_t *out, cpk_object_t *obj) {
    cpk_write_string(out, CPK_SYMBOL_STR);
    cpk_write_string(out, " ");

    if(CPK_IS_KEYWORD(obj->header))
        cpk_write_string(out, CPK_KEYWORD_STR);
    else
        explain_object_r(out, obj->symbol.package);

    cpk_write_string(out, " ");
    explain_object_r(out, obj->symbol.name);
}

static void explain_object_r(cpk_output_t *out, cpk_object_t *obj) {
    if(!obj) return;

    cpk_write_string(out, "(");
    switch(cpk_decode_header(obj->header)) {
        case CPK_BOOL: explain_bool(out, obj); break;
        case CPK_NUMBER: explain_number(out, obj); break;
        case CPK_STRING: explain_string(out, obj); break;
        case CPK_CONTAINER: explain_container(out, obj); break;
        case CPK_REF:
        case CPK_TAG:
        case CPK_INDEX:
            explain_ref(out, obj);
            break;
        case CPK_REMOTE_REF: explain_rref(out, obj); break;
        case CPK_CONS: explain_cons(out, obj); break;
        case CPK_PACKAGE: explain_package(out, obj); break;
        case CPK_SYMBOL: explain_symbol(out, obj); break;
        default:
            cpk_write_string(out, "Bad header: ");
            cpk_snprintf(out, 4, "%.3d", obj->header);
    }
    cpk_write_string(out, ")");
}

void cpk_explain_object(cpk_output_t *out, cpk_object_t *obj) {
    if(!out || !obj) return;

    explain_object_r(out, obj);
}
