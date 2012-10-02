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
#include <inttypes.h>

void cpk_print_buffer(cpk_output_t *out) {
    int i;
    
    printf("#(");
    for(i = 0; i < (out->buffer_used - 1); i++)
        printf("%u ", (unsigned int)(out->buffer[i]));

    printf("%u)\n", out->buffer[out->buffer_used - 1]);
}

int main() {
    cpk_output_t out;
    cpk_object_t obj, a, b;
    int i, j;

    cpk_output_init(&out);

    obj.header = CPK_SYMBOL;
    obj.symbol.name = &a;
    obj.symbol.package = &b;

    a.header = CPK_STRING | CPK_SIZE_8;
    a.string.size = sizeof("name") - 1;
    a.string.data = "name";

    b.header = CPK_STRING | CPK_SIZE_8;
    b.string.size = sizeof("package") - 1;
    b.string.data = "package";
        
    cpk_explain_object(&out, &obj);
    cpk_print(&out);
}

cpk_object_t* cpk_decode_r(cpk_input_t *in) {
    cpk_object_t *obj = calloc(1, sizeof(cpk_object_t)),
                 *tmp = NULL;
    uint32_t i = 0;

    cpk_decode(in, obj);
    if(CPK_IS_ERROR(tmp->header))
        goto error;

    switch(cpk_decode_header(obj->header)) {
        case CPK_REMOTE_REF:
            tmp = cpk_decode_r(in);
            if(CPK_IS_ERROR(tmp->header))
                goto error;
            
            obj->rref.val = tmp;
            break;

        case CPK_CONS:
            tmp = cpk_decode_r(in);
            if(CPK_IS_ERROR(tmp->header))
                goto error;

            obj->cons.car = tmp;

            tmp = cpk_decode_r(in);
            if(CPK_IS_ERROR(tmp->header))
                goto error;

            obj->cons.cdr = tmp;
            break;

        case CPK_CONTAINER:
            obj->container.obj = calloc(obj->container.size,
                                        sizeof(cpk_object_t*));
            for(i = 0; i < obj->container.size; i++) {
                tmp = cpk_decode_r(in);
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
