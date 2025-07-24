/**
   Copyright 2025 Achim Pieters | StudioPieters®

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   for more information visit https://www.studiopieters.nl
 **/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "form_urlencoded.h"


char *url_unescape(const char *buffer, size_t size) {
        int len = 0;

        int ishex(int c) {
                c = toupper(c);
                return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F');
        }

        int hexvalue(int c) {
                c = toupper(c);
                if ('0' <= c && c <= '9')
                        return c - '0';
                else
                        return c - 'A' + 10;
        }

        int i = 0, j;
        while (i < size) {
                len++;
                if (buffer[i] == '%') {
                        i += 3;
                } else {
                        i++;
                }
        }

        char *result = malloc(len+1);
        i = j = 0;
        while (i < size) {
                if (buffer[i] == '+') {
                        result[j++] = ' ';
                        i++;
                } else if (buffer[i] != '%') {
                        result[j++] = buffer[i++];
                } else {
                        if (i+2 < size && ishex(buffer[i+1]) && ishex(buffer[i+2])) {
                                result[j++] = hexvalue(buffer[i+1])*16 + hexvalue(buffer[i+2]);
                                i += 3;
                        } else {
                                result[j++] = buffer[i++];
                        }
                }
        }
        result[j] = 0;
        return result;
}


form_param_t *form_params_parse(const char *s) {
        form_param_t *params = NULL;

        int i = 0;
        while (1) {
                int pos = i;
                while (s[i] && s[i] != '=' && s[i] != '&') i++;
                if (i == pos) {
                        i++;
                        continue;
                }

                form_param_t *param = malloc(sizeof(form_param_t));
                param->name = url_unescape(s+pos, i-pos);
                param->value = NULL;
                param->next = params;
                params = param;

                if (s[i] == '=') {
                        i++;
                        pos = i;
                        while (s[i] && s[i] != '&') i++;
                        if (i != pos) {
                                param->value = url_unescape(s+pos, i-pos);
                        }
                }

                if (!s[i])
                        break;
        }

        return params;
}


form_param_t *form_params_find(form_param_t *params, const char *name) {
        while (params) {
                if (!strcmp(params->name, name))
                        return params;
                params = params->next;
        }

        return NULL;
}


void form_params_free(form_param_t *params) {
        while (params) {
                form_param_t *next = params->next;
                if (params->name)
                        free(params->name);
                if (params->value)
                        free(params->value);
                free(params);

                params = next;
        }
}
