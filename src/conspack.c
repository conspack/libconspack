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
    cpk_input_t in;
    cpk_object_t *obj;
    uint8_t data[] = { 36, 3, 20, 1, 2, 3 };

    cpk_output_init(&out);
    cpk_input_init(&in, data, sizeof(data));

    obj = cpk_decode_r(&in);
    
    cpk_explain_object(&out, obj);
    cpk_print(&out);
}

