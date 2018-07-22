/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Bart Van Der Meerssche <bart@flukso.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>
#include <stdarg.h>
#include "../mini-printf/mini-printf.h"
#include "usb.h"
#include "log.h"

static enum log_lvl log_mask = LOG_LVL_TRACE;
static char *topic2str[] = {
	"button",
	"ncn"
};

void log_write(enum log_topic topic, enum log_lvl lvl, const char *fmt, ...)
{
	char str[LOG_STR_MAX] = "[";
	size_t topic_len, str_len;
	va_list ap;

	if (lvl < log_mask) {
		return;
	}
	topic_len = strlen(topic2str[topic]);
	strcpy(str + 1, topic2str[topic]);
	strcpy(str + 1 + topic_len, "] ");
	va_start(ap, fmt);
	vsnprintf(str + 3 + topic_len, LOG_STR_MAX - 3 - topic_len, fmt, ap);
	va_end(ap);
	str_len = strlen(str);
	snprintf(str + str_len, LOG_STR_MAX - str_len, "\r\n");
	usb_print_log(str);
}
