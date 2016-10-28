#ifndef CLI_H_
#define CLI_H_

#include <stdbool.h>

#define CLI_MMIO32(addr)	(*(volatile unsigned int *)(addr))

typedef int (*cli_fun)(int argc, const char * const *argv);

struct cli_cmd_s {
	char *name;
	int arity;
	char *description;
	bool hidden;
	cli_fun exec;
};

int cli_execute(int argc, const char * const *argv);
char ** cli_complete(int argc, const char * const *argv);
void cli_sigint(void);
void cli_insert_char(char ch);
void cli_setup(void);

#endif
