#ifndef CLI_H_
#define CLI_H_

int cli_execute(int argc, const char * const *argv);
char ** cli_complete(int argc, const char * const *argv);
void cli_sigint(void);
void cli_insert_char(char ch);
void cli_setup(void);

#endif
