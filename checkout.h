#include <stddef.h>
#include <allocate.h>

#ifndef CHECKOUT_H
#define CHECKOUT_H

struct checkout{
	struct checkout * next_item;

	struct checkout * next_patron;

	//An xor of the item and patron that forms the checkout
	//Assigned as a size_t so I can xor it easily
	size_t item_patron;
};

void unallocate_checkouts(struct checkout * c);

#endif
