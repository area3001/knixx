#ifndef NCN_H_
#define NCN_H_

#include <stddef.h>
#include <stdbool.h>

#define NCN_TX_BUFFER_SIZE	263

struct ncn_tx_buffer {
	char data[NCN_TX_BUFFER_SIZE];
	bool empty;
	size_t idx;
	size_t fill;
};

void ncn_setup(void);
bool ncn_tx(char *frame, size_t len);

#endif
