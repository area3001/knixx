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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../microrl/src/microrl.h"
#include "../mini-printf/mini-printf.h"
#include "usb.h"
#include "ncn.h"
#include "cli.h"

microrl_t rl;

static int cli_banner(void)
{
	usb_print_console(
		" __          .__\r\n"
		"|  | __ ____ |__|__  ______  ___\r\n"
		"|  |/ //    \\|  \\  \\/  /\\  \\/  /\r\n"
		"|    <|   |  \\  |>    <  >    < \r\n"
		"|__|_ \\___|  /__/__/\\_ \\/__/\\_ \\\r\n"
		"     \\/    \\/         \\/      \\/\r\n"
		__DATE__"     "KNIXX_COMMIT"\r\n");
	return 0;
}

static int cli_hello(int argc, const char * const *argv)
{
	(void)argc;
	(void)argv;

	usb_print_console("world!\r\n");
	return 0;
}

static int cli_help(int argc, const char * const *argv);

static int cli_peek(int argc, const char * const *argv) {
	(void)argc;
	unsigned int i, j, addr, lines;
	char hex[CLI_HEX_LENGTH];

	addr = strtoul(argv[1], NULL, 0);
	lines = strtoul(argv[2], NULL, 0);
	for (i = 0; i < lines; i++) {
		usb_print_console("0x");
		snprintf(hex, CLI_HEX_LENGTH, "%08x ", addr + i * 16);
		usb_print_console(hex);
		for (j = 0; j < 4; j++) {
			snprintf(hex, CLI_HEX_LENGTH, " %08x",
					CLI_MMIO32(addr + ((i * 4) + j) * 4));
			usb_print_console(hex);
		}
		usb_print_console("\r\n");
	}
	return 0;
}

static int cli_poke(int argc, const char * const *argv) {
	(void)argc;
	unsigned int addr, value;

	addr = strtoul(argv[1], NULL, 0);
	value = strtoul(argv[2], NULL, 0);
	*(unsigned int *)addr = value;
	return 0;
}

static int cli_ncn(int argc, const char * const *argv) {
	(void)argc;
	char byte;

	byte = strtoul(argv[1], NULL, 0);
	ncn_tx(&byte, 1);
	return 0;
}

static int cli_a7ea(int argc, const char * const *argv);

static struct cli_cmd_s cli_cmd[] = { {
	.name = "hello",
	.arity = 0,
	.description = "world!",
	.hidden = true,
	.exec = cli_hello
}, {
	.name = "help",
	.arity = 0,
	.description = "this help",
	.hidden = false,
	.exec = cli_help
}, {
	.name = "a7ea",
	.arity = 0,
	.description = "this hidden help",
	.hidden = true,
	.exec = cli_a7ea
}, {
	.name = "peek",
	.arity = 2,
	.description = "<addr> <lines> : raw memory dump",
	.hidden = true,
	.exec = cli_peek
}, {
	.name = "poke",
	.arity = 2,
	.description = "<addr> <value> : set *addr to value",
	.hidden = true,
	.exec = cli_poke
}, {
	.name = "ncn",
	.arity = 1,
	.description = "<byte> : tx byte on ncn uart",
	.hidden = true,
	.exec = cli_ncn
}, {
	.name = NULL /* sentinel */
} };

static int cli_help(int argc, const char * const *argv)
{
	(void)argc;
	(void)argv;
	int i = 0;

	while (cli_cmd[i].name != NULL) {
		if (!cli_cmd[i].hidden) {
			usb_print_console(cli_cmd[i].name);
			usb_print_console("\t");
			usb_print_console(cli_cmd[i].description);
			usb_print_console("\r\n");
		}
		i++;
	}
	return 0;
}

static int cli_a7ea(int argc, const char * const *argv)
{
	(void)argc;
	(void)argv;
	int i = 0;

	while (cli_cmd[i].name != NULL) {
		if (cli_cmd[i].hidden) {
			usb_print_console(cli_cmd[i].name);
			usb_print_console("\t");
			usb_print_console(cli_cmd[i].description);
			usb_print_console("\r\n");
		}
		i++;
	}
	return 0;
}

int cli_execute(int argc, const char * const *argv)
{
	static bool init = false;
	int i = 0;

	usb_print_console("\r\n");
	if (argc == 0) {
		if (!init) {
			init = true;
			return cli_banner();
		} else {
			return 0;
		}
	}
	while (cli_cmd[i].name != NULL) {
		if (strcmp(cli_cmd[i].name, argv[0]) == 0 &&
		    cli_cmd[i].arity == argc - 1) {
			return cli_cmd[i].exec(argc, argv);
		}
		i++;
	};
	return cli_help(0, NULL);
}

char ** cli_complete(int argc, const char * const *argv)
{
	(void)argc;
	(void)argv;

	return NULL;
}

void cli_sigint(void)
{
}

void cli_insert_char(char ch)
{
	microrl_insert_char(&rl, ch);
}

void cli_setup(void)
{
	microrl_init(&rl, usb_print_console);
	microrl_set_execute_callback(&rl, cli_execute);
#ifdef _USE_COMPLETE
	microrl_set_complete_callback(&rl, cli_complete);
#endif
	microrl_set_sigint_callback(&rl, cli_sigint);
}
