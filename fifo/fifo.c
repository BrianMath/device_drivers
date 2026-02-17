#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/ioccom.h>
#include "fifo.h"

MALLOC_DEFINE(M_FIFO, "fifo_buffer", "buffer for queue driver");

// Forward declarations
static d_open_t  fifo_open;
static d_close_t fifo_close;
static d_read_t  fifo_read;
static d_write_t fifo_write;
static d_ioctl_t fifo_ioctl;

// Character device switch table
static struct cdevsw fifo_cdevsw = {
	.d_version = D_VERSION,
	.d_name    = "fifo",
	.d_open    = fifo_open,
	.d_close   = fifo_close,
	.d_read    = fifo_read,
	.d_write   = fifo_write,
	.d_ioctl   = fifo_ioctl
};

typedef struct node {
	char value[64];
	struct node *next;
} Node;

typedef struct head {
	ull size;
	Node *start;
	Node *end;
} Head;

static Head *fifo;
static struct cdev *fifo_dev;

static int
fifo_open(struct cdev *dev, int oflags, int devtype, struct thread *td) {
	uprintf("Opening fifo device.\n");
	return 0;
}

static int
fifo_close(struct cdev *dev, int fflag, int devtype, struct thread *td) {
	uprintf("Closing fifo device.\n");
	return 0;
}

static int
fifo_write(struct cdev *dev, struct uio *uio, int ioflag) {
	int error = 0;
	int amount = uio->uio_resid;

	if (amount == 0) {
		return error;
	}

	// Allocate a new node
	Node *new_node = (Node *) malloc(sizeof(Node), M_FIFO, M_WAITOK);
	new_node->next = NULL;
	
	// Move received value to the new node 
	error = uiomove(new_node->value, amount, uio);
	if (error != 0) {
		uprintf("Write failed\n");
	}
	new_node->value[amount] = '\0';

	if (fifo->start == NULL) {
		fifo->start = new_node;
		fifo->end = new_node;
	} else {
		fifo->end->next = new_node;
		fifo->end = new_node;
	}

	fifo->size++;

	return error;
}

static int
fifo_read(struct cdev *dev, struct uio *uio, int ioflag) {
	int error = 0;
	//int amount = uio->uio_resid;

	//if (amount == 0) {
	//	return error;
	//}
	
	uprintf("Head -> ");

	Node *aux = fifo->start;
	while (aux != NULL) {
		// error = uiomove(aux->value, amount, uio);

		//if (error != 0) {
		//	uprintf("Read failed\n");
		//	return error;
		//}

		uprintf("%s -> ", aux->value);

		aux = aux->next;
	}

	uprintf("NULL\n");

	return error;
}

static int
fifo_clear(Head *fifo) {
	int error = 0;

	while (fifo->start != NULL) {
		Node *aux = fifo->start->next;
		free(fifo->start, M_FIFO);
		fifo->start = aux;
	}
	fifo->size = 0;

	return error;
}

static int
fifo_pop(Head *fifo) {
	int error = 0;

	if (fifo->size == 0) {
		uprintf("Fifo is empty\n");
		return error;
	}

	Node *aux = fifo->start;
	uprintf("Pop: %s\n", aux->value);
	fifo->start = aux->next;
	free(aux, M_FIFO);
	fifo->size--;

	return error;
}

static int
fifo_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int fflag, struct thread *td) {
	int error = 0;

	switch (cmd) {
	case FIFO_CLEAR:
		fifo_clear(fifo);
		uprintf("Fifo cleared\n");
		break;

	case FIFO_GET_SIZE:
		uprintf("Fifo size: %llu\n", fifo->size);
		break;

	case FIFO_POP:
		fifo_pop(fifo);
		break;

	default:
		error = ENOTTY;
		break;
	}

	return error;
}

static int
fifo_modevent(module_t mod __unused, int event, void *arg __unused) {
	int error = 0;

	switch (event) {
	case MOD_LOAD:
		fifo = (Head *) malloc(sizeof(Head), M_FIFO, M_WAITOK);
		fifo->size = 0;
		fifo->start = NULL;
		fifo->end = NULL;

		fifo_dev = make_dev(&fifo_cdevsw, 0, UID_ROOT, GID_WHEEL,
				0666, "fifo");
		uprintf("Fifo driver loaded.\n");
		break;

	case MOD_UNLOAD:
		destroy_dev(fifo_dev);
		fifo_clear(fifo);
		free(fifo, M_FIFO);
		uprintf("Fifo driver unloaded.\n");
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}

	return error;
}

DEV_MODULE(fifo, fifo_modevent, NULL);

