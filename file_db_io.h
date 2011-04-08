//#include "allocate.h"
#include "patron.h"
#include "item.h"
#include "stdio.h"

struct patron * load_patron(char * filename);

struct item * load_item(char * filename);

int remove_whitespace(char * line);

void advance_to_newline(FILE * file);
