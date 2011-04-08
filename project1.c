#include <allocate.h>
#include "file_db_io.h"
#include "checkout.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	if(argc >= 3)
	{
		char * patron_file = argv[1];
		char * item_file = argv[2];

		//Loads the patron list
		struct patron * first_patron = load_patron(patron_file);
	
		//Load patron will return null if there was an error, so exit
		if(first_patron == NULL)
			exit(0);
		//Load item list
		struct item * first_item = load_item(item_file);

		//Load item will return null if there was an error, so deallocate and exit
		if(first_item == NULL)
		{
			unallocate_patrons(first_patron);
			exit(0);
		}

		char * command = allocate(256);

		//Memory error, unallocate and exit
		if(command == NULL)
		{
			unallocate_patrons(first_patron);
			unallocate_items(first_item);
			printf("Memory allocation failed!\n");
			exit(0);
		}
	
		printf("\n? ");

		while(fgets(command, 256 , stdin))
		{
			printf("\n");

			char p_c_id;
			short p_s_id;

			short c_f_id;
			short c_s_id;
		
			
			if( sscanf(command , "borrow %c%hd %hd.%hd", &p_c_id , &p_s_id , &c_f_id , &c_s_id) == 4)
			{
				struct patron * p = NULL;
				struct item * i = NULL;
				
				//Checks to make sure the patron and item exists, and they are able to check out
				if((p = find_patron(first_patron , p_c_id , p_s_id)) == NULL)
					printf("%c%04hd does not exist\n" , p_c_id , p_s_id);
				else if((i = find_item(first_item , c_f_id , c_s_id)) == NULL)
					printf("%hd.%hd does not exist\n" , c_f_id , c_s_id);
				else if(count_item(i) + 1 > i->copies)
					printf("no more copies of %hd.%hd are available\n" , c_f_id , c_s_id);
				else if(count_patron(p) + 1 > 7)
					printf("%c%04hd cannot check out any more items\n" , p_c_id , p_s_id);
				else if(has_checkout(p , c_f_id , c_s_id))
					printf("%c%04hd already has %hd.%hd checked out\n" , p_c_id , p_s_id , c_f_id , c_s_id);
				else
				{
					struct checkout * c = allocate(sizeof(struct checkout));

					//Memory allocation error
					if(c == NULL)
					{
						unallocate(command);
						unallocate_patrons(first_patron);
						unallocate_items(first_item);
						printf("Memory allocation failed!\n");
						exit(0);
					}

					//Item Patron is an xor of the patron and item pointer, so we can easily
					//Go from between items and patrons in a checkout
					c->item_patron = (size_t)p ^ (size_t)i;
					c->next_item = NULL;
					c->next_patron = NULL;

					//We need to borrow from both so the checkouts get linked up to the
					//Previous item/patron checkouts
					borrow_patron(c , p);
					borrow_item(c , i);
				}	
			}
			else if( sscanf(command, "return %c%hd %hd.%hd", &p_c_id, &p_s_id , &c_f_id, &c_s_id) == 4)
			{
				struct patron * p = NULL;
				struct item * i = NULL;
			
				//Checks to make sure the items and patron exists
				if((p = find_patron(first_patron , p_c_id , p_s_id)) == NULL)
					printf("%c%04hd does not exist\n" , p_c_id , p_s_id);
				else if((i = find_item(first_item , c_f_id , c_s_id)) == NULL)
					printf("%hd.%hd does not exist\n", c_f_id , c_s_id);
				else
				{
					//We return from both so they are correctly removed from
					//Both the item and patrons checkouts
					struct checkout * c_p = return_patron(i , p);
					struct checkout * c_i = return_item(p , i);

					//The return functions will return the pointer of the checkout that is returned
		
					if(c_p != NULL && c_i != NULL)
					{
						//These are just for pointer sanity
						c_p->next_item = NULL;
						c_p->next_patron = NULL;
						//Unallocate the checkout
						unallocate(c_p);
					}
					else
						printf("%c%04hd does not have %hd.%hd checked out\n" , p_c_id , p_s_id , c_f_id , c_s_id);
				}

			}
			else if( sscanf(command, "available %hd.%hd" , &c_f_id, &c_s_id) == 2)
			{
				struct item * i = find_item(first_item , c_f_id , c_s_id);
	
				if(i == NULL)
					printf("%hd.%hd does not exist\n" , c_f_id , c_s_id);
				else
				{
					//Count item returns the number of checkouts from an item
					int num_out = count_item(i); 

					printf("Item ");
					print_item(i);
					printf(": %d of %hd copies available\n", i->copies - num_out, i->copies);
				}
			}
			else if( sscanf(command , "out %hd.%hd" , &c_f_id , &c_s_id) == 2)
			{
				struct item * i = find_item(first_item , c_f_id , c_s_id);
	
				if( i == NULL)
					printf("%hd.%hd does not exist\n", c_f_id , c_s_id);
				else
					out_item(i);
			}
			else if ( sscanf( command , "out %c%4hd", &p_c_id , &p_s_id) == 2)
			{
				struct patron * p = find_patron(first_patron , p_c_id , p_s_id);

				if (p == NULL)
					printf("%c%04hd does not exist\n" , p_c_id , p_s_id);			
				else
					out_patron(p);
			}

			printf("\n? ");

			if(strchr(command , '\n') == NULL)
				advance_to_newline(stdin);
		}
	
		unallocate(command);


		//Print the item and patron lists
		struct item * i = first_item;

		while(i != NULL)
		{
			printf("\n");
			out_item(i);

			i = i->next_item;
		}	

		struct patron * p = first_patron;

		while(p != NULL)
		{
			printf("\n");
			out_patron(p);
	
			p = p->next_patron;
		}

		//These functions will unallocate all checkouts, patrons and items
		unallocate_patrons(first_patron);
		unallocate_items(first_item);
	}
	
	if(argc < 2)
		printf("Patron database file error\n");
		
	if(argc < 3)
		printf("Item database file error\n");	
}
