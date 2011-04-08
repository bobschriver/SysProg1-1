#include "checkout.h"
#include <allocate.h>

void unallocate_checkouts(struct checkout * c)
{
	//Unallocate all checkouts linked to the parameter checkout
	while(c != NULL)
	{
		struct checkout * d_c = c;
		c = c->next_item;
		
		unallocate(d_c);
	}
}
