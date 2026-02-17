#include <sys/types.h>
#include <sys/ioctl.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fifo.h"

static enum {UNSET, CLEAR, GETSIZE, POP} action = UNSET;

static void usage() {
	/*
	* Arguments for this program are "either-or".
	* That is, 'fifo-config -c' and 'echo_config -s' are valid.
	* However, 'fifo_config -c -s' is invalid.
	*/
	fprintf(stderr, "usage: fifo_config (-c | -s | -p)\n");
	exit(1);
}

/*
* This program clears or gets the size of the fifo
* found in /dev/fifo.
*/
int main(int argc, char *argv[]) {
	int ch, fd, i;

	/*
	* Parse the command-line argument list to determine
	* the correct course of action.
	*
	* -c: clear the fifo, that is, free all nodes
	* -s: get the fifo size, that is, number of nodes
	*/
	while ((ch = getopt(argc, argv, "csp")) != -1) {
		switch (ch) {
		case 'c':
			if (action != UNSET) {
				usage();
			}
			action = CLEAR;
			break;

		case 's':
			if (action != UNSET) {
				usage();
			}
			action = GETSIZE;
			break;

		case 'p':
			if (action != UNSET) {
				usage();
			}
			action = POP;
			break;

		default:
			usage();
			break;
		}
	}

	/*
	* Perform the chosen action.
	*/
	if (action == CLEAR) {
		fd = open("/dev/fifo", O_RDWR);
		if (fd < 0) {
			err(1, "open(/dev/fifo)");
		}

		i = ioctl(fd, FIFO_CLEAR, NULL);
		if (i < 0) {
			err(1, "ioctl(/dev/fifo)");
		}

		close(fd);
	} else if (action == GETSIZE) {
		fd = open("/dev/fifo", O_RDWR);
		if (fd < 0) {
			err(1, "open(/dev/fifo)");
		}

		i = ioctl(fd, FIFO_GET_SIZE, NULL);
		if (i < 0) {
			err(1, "ioctl(/dev/fifo)");
		}

		close(fd);
	} else if (action == POP) { 
		fd = open("/dev/fifo", O_RDWR);
		if (fd < 0) {
			err(1, "open(/dev/fifo)");
		}

		i = ioctl(fd, FIFO_POP, NULL);
		if (i < 0) {
			err(1, "ioctl(/dev/fifo)");
		}

		close(fd);
	} else {
		usage();
	}

	return 0;
}

