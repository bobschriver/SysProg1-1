#include "patron.h"
#include <stdio.h>
#include <string.h>
#include <allocate.h>

void borrow_patron(struct checkout * c, struct patron * p)
{
	struct item * i  = (struct item *)(c->item_patron ^ (size_t)p);

	if(p->next_checkout != NULL)
	{
		//Check if the checkout should be the checkout referenced by p->next_checkout
		if(compare_item(i , (struct item *)(p->next_checkout->item_patron ^ (size_t)p)) < 0)
		{
			c->next_item = p->next_checkout;
			p->next_checkout = c;

			
		}
		else
		{
			//Else find the checkout where the next checkout has an item greater than the checkout we are trying to insert
			struct checkout * insert = p->next_checkout;
			
			while(insert->next_item != NULL && compare_item(i , (struct item *)(insert->next_item->item_patron ^ (size_t)p)) > 0)
		 	{
				insert = insert->next_item;
		 	}

			//Assign the next items so we insert the checkout into the middle of the list
		 	c->next_item = insert->next_item;
		 	insert->next_item = c;
		 }
	}
	else
		p->next_checkout = c;
}

struct checkout * return_patron(struct item * i, struct patron * p)
{
	struct checkout * c = p->next_checkout;
	struct checkout * prev_checkout = NULL;
	
	//First, find the checkout previous to the one we want to return
	while(c != NULL && (struct item *)(c->item_patron ^ (size_t)p) != i)
	{
		prev_checkout = c;
		c = c->next_item;
	}
	
	//Now, reassign the next_item pointers to go around the returned checkouts
	if(c != NULL)
	{
		if(prev_checkout != NULL)
			prev_checkout->next_item = c->next_item;
		else p->next_checkout = c->next_item;
	}

	return c;
}

struct patron * find_patron(struct patron * first_patron , char c_id , short s_id)
{
	struct patron * p = first_patron;
	
	//Iterate through the patron list until we find a patron with the matching ID, else return NULL
	while(p != NULL && (p->c_id != c_id || p->s_id != s_id))
	{
		p = p->next_patron;
	}

	return p;
}

void out_patron(struct patron * p)
{
	struct checkout * c = p->next_checkout;

	printf("Patron %c%04hd (%s) has" , p->c_id , p->s_id , p->name);

	if(count_patron(p) > 0)
		printf(" these items checked out:\n");
	else
		printf(" no items checked out\n");

	while(c != NULL)
	{
		//Get the item by xoring the patron pointer with the item_patron 'pointer'
		struct item * i = (struct item *)(c->item_patron ^ (size_t)p);
		
		printf("   ");
		print_item(i);
		printf("\n");

		c = c->next_item;
	}
}

int count_patron(struct patron * p)
{
	//Counts all items checked out byt a patron

	int count = 0;

	struct checkout * c = p->next_checkout;

	while(c != NULL)
	{
		count++;

		c = c->next_item;
	}

	return count;
}

int compare_patron(struct patron * first , struct patron * second)
{
	//Compares two patrons as defined in the project guideline

	int diff = strcmp(first->name , second->name);
	
	if(diff != 0)
		return diff;

	diff = first->c_id - second->c_id;

	if(diff != 0)
		return diff;
	
	diff = first->s_id - second->s_id;

	return diff;
}

void print_patron(struct patron * p)
{
	printf("%c%04hd (%s)" , p->c_id , p->s_id , p->name);
}

int has_checkout(struct patron * p , short f_id , short s_id)
{

	//Check if a patron has checked out an item by looking through all the checkouts
	//And seeing if the item you are checking is contained in  them
	struct checkout * c = p->next_checkout;

	while(c != NULL)
	{
		//Get the item pointer by xoring with the patron and the item_patron 'pointer'
		struct item * i = (struct item *)(c->item_patron ^ (size_t)p);

		if(i->f_id == f_id && i->s_id == s_id)
			return 1;

		c = c->next_item;
	}

	return 0;
}

struct patron * unallocate_patron(struct patron * p)
{
	struct patron * ret = p->next_patron;

	//We only unallocate checkouts from patron, not items
	unallocate_checkouts(p->next_checkout);
	//Pointer sanity, probably unnessecary
	p->next_checkout = NULL;
	
	unallocate(p->name);
	unallocate(p);

	return ret;
}

void unallocate_patrons(struct patron * first_patron)
{
	//Unallocate all patrons
	while(first_patron != NULL)
		first_patron = unallocate_patron(first_patron);
}
