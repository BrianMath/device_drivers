#ifndef FIFO_H
#define	FIFO_H

// ioctl commands
#define FIFO_CLEAR    _IO('F', 1)
#define FIFO_GET_SIZE _IO('F', 2)
#define FIFO_POP      _IO('F', 3)

#define ull unsigned long long

#endif
