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

void cpk_print_buffer(cpk_output_t *out) {
    int i;
    
    printf("#(");
    for(i = 0; i < (out->buffer_used - 1); i++)
        printf("%u ", (unsigned int)(out->buffer[i]));

    printf("%u)\n", out->buffer[out->buffer_used - 1]);
}

int main() {
    cpk_output_t out;
    cpk_input_t in;
    cpk_object_t obj;
    int container_elements = 3, i;

    cpk_output_init(&out);

    cpk_encode_container(&out, CPK_CONTAINER_VECTOR, container_elements, 0);
    for(i = 0; i < container_elements; i++) {
        cpk_write8(&out, CPK_NUMBER | CPK_INT8);
        cpk_write8(&out, i);
    }

    cpk_print_buffer(&out);
    cpk_output_clear(&out);

    cpk_encode_container(&out, CPK_CONTAINER_VECTOR, container_elements,
                         CPK_NUMBER | CPK_INT8);
    for(i = 0; i < container_elements; i++)
        cpk_write8(&out, i);

    cpk_print_buffer(&out);
    cpk_output_clear(&out);

    cpk_encode_string(&out, "hello world");
    cpk_print_buffer(&out);
    cpk_output_clear(&out);

    cpk_write8(&out, CPK_NUMBER | CPK_DOUBLE_FLOAT);
    cpk_write_double(&out, -100.01);
    cpk_print_buffer(&out);

    cpk_input_init(&in, out.buffer, out.buffer_used);
    cpk_decode(&in, &obj);

    printf("Header: %hd\n", (short)obj.header);

    if(CPK_IS_NUMBER(obj.header)) {
        printf("Is number\n");
        if(CPK_NUMBER_TYPE(obj.header) == CPK_DOUBLE_FLOAT)
            printf("Is double: %lf\n", obj.number.val.double_float);
    }

    cpk_free(&obj);
    cpk_output_clear(&out);

    cpk_output_fini(&out);
}
