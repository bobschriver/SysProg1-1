#ifndef ITEM_H
#define ITEM_H

#include "checkout.h"
#include "patron.h"

struct item {
	char * author;
	char * title;
	
	short f_id;
	short s_id;

	short copies;

	struct checkout * next_checkout;

	struct item * next_item;
};

void borrow_item(struct checkout * c, struct item * i);

struct checkout * return_item(struct patron * p, struct item * i);

struct item * find_item(struct item * first_item , short f_id , short s_id);

void out_item(struct item * i);

int count_item(struct item * i);

void print_item(struct item * i);

int compare_item(struct item * first, struct item * second);

struct item * unallocate_item(struct item * i);

void  unallocate_items(struct item * first_item);
#endif
