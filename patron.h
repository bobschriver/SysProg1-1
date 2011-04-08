#ifndef PATRON_H
#define PATRON_H

#include "checkout.h"
#include "item.h"

struct patron{
	char * name;
	
	char c_id;
	short s_id;

	struct patron * next_patron;

	struct checkout * next_checkout;
};

void borrow_patron(struct checkout * c, struct patron * p);

//
struct checkout * return_patron(struct item * i, struct patron * p);

void out_patron(struct patron * p);

int count_patron(struct patron * p);

struct patron * find_patron(struct patron * first_patron, char c_id , short s_id);

void print_patron(struct patron * p);

int compare_patron(struct patron * first , struct patron * second);

int has_checkout(struct patron * p , short f_id , short s_id);

struct patron * unallocate_patron(struct patron * p);

void unallocate_patrons(struct patron * first_patron);
#endif
