#include "item.h"
#include <stdio.h>
#include <allocate.h>

void borrow_item(struct checkout * c, struct item * i)
{
	//Get the patron pointer by xoring the item pointer and the item_patron 'pointer'
	struct patron * p = (struct patron *)(c->item_patron ^ (size_t)i);

	if(i->next_checkout != NULL)
	{

		//See if the checkout should be inserted as i->next_checkout
		if(compare_patron(p , (struct patron *)(i->next_checkout->item_patron ^ (size_t)i)) < 0)
		{
			c->next_patron = i->next_checkout;
			i->next_checkout = c;
		}
		else
		{
			struct checkout * insert = i->next_checkout;

			//Insert pointer is where the next item is greater than the item at the checkout to be inserted
			while(insert->next_patron != NULL && compare_patron(p , (struct patron *)(insert->next_patron->item_patron ^ (size_t)i)) > 0)
			{
				insert = insert->next_patron;
			}


			//Insert the checkout in the singly linked list
			c->next_patron = insert->next_patron;

			insert->next_patron = c;
		}
	}
	else
		i->next_checkout = c;
}

struct checkout * return_item(struct patron * p, struct item * i)
{
	struct checkout * c = i->next_checkout;
	struct checkout * prev_checkout = NULL;

	//Find the checkout previous to the one we are trying to return
	while(c != NULL && (struct patron *)(c->item_patron ^ (size_t)i) != p)
	{
		prev_checkout = c;
		c = c->next_patron;
	}

	//Remove the item from the list
	if(c != NULL)
	{
		if(prev_checkout != NULL)
			prev_checkout->next_patron = c->next_patron;
		else i->next_checkout = c->next_patron;
	}

	return c;
}

struct item * find_item(struct item * first_item, short f_id , short s_id)
{
	struct item * i = first_item;

	//Go through the list until we find an item that matches the id of the item 
	//We are tryin to find. If we can't find it, return NULL
	while(i != NULL && (i->f_id != f_id || i->s_id != s_id))
	{
		i = i->next_item;
	}

	return i;
}

void out_item(struct item * i)
{
	struct checkout * c = i->next_checkout;

	printf("Item %hd.%hd (%s/%s) is" , i->f_id , i->s_id , i->author , i->title);

	if(count_item(i) > 0)
		printf(" checked out to:\n");
	else
		printf(" not checked out\n");

	//Print all of the patrons which have checkout out the item
	while(c != NULL)
	{
		struct patron * p = (struct patron *)(c->item_patron ^ (size_t)i);

		printf("   ");
		print_patron(p);
		printf("\n");
	
		c = c->next_patron;
	}
}

int count_item(struct item * i)
{
	int count = 0;

	struct checkout * c = i->next_checkout;

	while(c != NULL)
	{
		count ++;
		c = c->next_patron;
	}

	return count;
}

int compare_item(struct item * first, struct item * second)
{

	//Compare item as defined in project specification
	int diff = strcmp(first->author, second->author);

	if(diff != 0)
		return diff;

	diff = strcmp(first->title , second->title);

	if(diff != 0)
		return diff;

	diff = (first->f_id * 1000 + first->s_id) - (second->f_id * 1000 + second->s_id);

	return diff;	
}

void print_item(struct item * i)
{
	printf("%hd.%hd (%s/%s)" , i->f_id , i->s_id , i->author , i->title); 
}

struct item * unallocate_item(struct item * i)
{
	struct item * ret = i->next_item;

	i->next_checkout = NULL;
	
	//Unallocate all dynamic memory in item
	unallocate(i->author);
	unallocate(i->title);
	unallocate(i);

	return ret;
}

void unallocate_items(struct item * first_item)
{
	//Unallocate all items
	while(first_item != NULL)
		first_item = unallocate_item(first_item);
}
