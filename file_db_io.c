#include <allocate.h>
#include <stdio.h>
#include "file_db_io.h"

struct patron * load_patron(char *filename)
{
	FILE * patron_file = fopen(filename , "r");

	struct patron * first_patron = NULL;
	
	while(patron_file != NULL)
	{
		struct patron * curr_patron = allocate(sizeof(struct patron));

		//Pointer sanity so these will always return NULL correctly
		curr_patron->next_patron = NULL;
		curr_patron->next_checkout = NULL;
	
		//Memory allocationg failure. gotos used for centralized errors
		if(curr_patron == NULL)
			goto memory_failure_1;

		char * name = allocate(40);

		if(name == NULL)
			goto memory_failure_2;
		
		//If we are unable to fget anymore, the file is empty
		if(fgets(name , 40 , patron_file) == NULL)
		{
			unallocate(curr_patron);
			unallocate(name);
			break;
		}
								
		//Remove whitespace will return false if no newline is found
		//IE if the name has been truncated by fgets
		//So we have to advance to the newline so the file pointer is correct for the next call
		if(!remove_whitespace(name))
			advance_to_newline(patron_file);


		//Reallocate the name to a smaller chunk of memory
		char * s_name = allocate(strlen(name) + 1);

		if(s_name == NULL)
			goto memory_failure_3;

		strcpy(s_name , name);
		
		unallocate(name);
	
		curr_patron->name  = s_name;
		
		fscanf(patron_file, "%c%hd\n" , &curr_patron->c_id , &curr_patron->s_id);
		
		if(curr_patron->c_id < 'A' || curr_patron->c_id > 'Z' 
			|| curr_patron->s_id > 9999)
		{
			fprintf(stderr , "Bad patron %c%04hd (%s)\n", curr_patron->c_id , curr_patron->s_id , curr_patron->name);
			//Goto used for centralized error reporting
			goto error;
		}

		if(first_patron != NULL)
		{
			//Check to see if a patron is a duplicate by finding it
			struct patron * dup_patron = find_patron(first_patron , curr_patron->c_id , curr_patron->s_id);

			if(dup_patron != NULL)
			{
				fprintf(stderr , "Patron %c%04hd (%s) duplicates that for (%s)\n" , curr_patron->c_id , curr_patron->s_id , curr_patron->name , dup_patron->name);
				goto error;
			}

			//Special case of comparison where the current patron should be the first patron
			if(compare_patron(curr_patron, first_patron) < 0)
			{
				curr_patron->next_patron = first_patron;
				first_patron = curr_patron;
				continue;
			}

			//Find where the current patron should be inserted in the list
			struct patron * insert_patron = first_patron;

			while(insert_patron->next_patron != NULL &&
				compare_patron(curr_patron , insert_patron->next_patron) > 0)
			{
				insert_patron = insert_patron->next_patron;
			}
			
			curr_patron->next_patron = insert_patron->next_patron;
			insert_patron->next_patron = curr_patron;
		}
		else
			first_patron = curr_patron;


		continue;
		//This just means there was an error where we should continue processing
		//Deallocate dynamic memory and continue
		error:
			unallocate(s_name);
			unallocate(curr_patron);
			continue;

		//Memory allocation failures. Clean up dynamic memory and return
		memory_failure_3:
			unallocate(name);
		memory_failure_2:
			unallocate(curr_patron);
		memory_failure_1:
			fprintf(stderr , "Memory allocation failed!\n");
			unallocate_patrons(first_patron);
			fclose(patron_file);
			return NULL;	
	}

	if(patron_file == NULL)
		printf("Patron database file error\n");
	else
		fclose(patron_file);

	return first_patron;
}

 
struct item * load_item(char * filename)
{
	FILE * item_file = fopen(filename , "r");
	
	struct item * first_item = NULL;

	while(item_file != NULL)
	{
		struct item * curr_item = allocate(sizeof(struct item));

		//Memory allocation failure
		if(curr_item == NULL)
			goto memory_failure_1;

		//Set these to NULL manually for pointer sanity		
		curr_item->next_item = NULL;
		curr_item->next_checkout = NULL;

		char * author = allocate(60);
		//Memory allocation failure
		if(author == NULL)
			goto memory_failure_2;

		//If we are unable to fget any more, we've reached the end of the file so free memory and continue
		if(fgets(author, 60, item_file) == NULL)
		{
			unallocate(curr_item);
			unallocate(author);
			break;
		}

		//Remove whitespace returns true if the string contains a newline
		//If it returns false, we have to advance the file to the newline for the next sscanf
		if(!remove_whitespace(author))
			advance_to_newline(item_file);

		//Reallocate author so it takes up less memory
		char * s_author = allocate(strlen(author) + 1);

		if(s_author == NULL)
			goto memory_failure_3;
	
		strcpy(s_author , author);

		unallocate(author);

		curr_item->author = s_author;
		
		//Same basic sequence as above
		char * title = allocate(60);

		if(title == NULL)
			goto memory_failure_4;
				
		fgets(title, 60, item_file);
	
		if(!remove_whitespace(title))
			advance_to_newline(item_file);
		
		char * s_title = allocate(strlen(title) + 1);

		if(s_title == NULL)
			goto memory_failure_5;

		strcpy(s_title , title);

		unallocate(title);

		curr_item->title = s_title;
		
		fscanf(item_file , "%hd.%hd\n" , &curr_item->f_id ,  &curr_item->s_id);
		
		//ID Error. Advance to newline to that we skip the next line and then go to the error label
		if(curr_item->f_id > 999 || curr_item->s_id > 999)
		{	
			fprintf(stderr , "Bad Item %hd.%hd (%s/%s)\n" , curr_item->f_id , curr_item->s_id , curr_item->author , curr_item->title);
			advance_to_newline(item_file);
			goto error;
		}

		fscanf(item_file , "%hd\n" , &curr_item->copies);
		
		//To many copies
		if(curr_item->copies > 99)
		{
			fprintf(stderr , "Bad Item %hd.%hd (%s/%s)\n" , curr_item->f_id , curr_item->s_id , curr_item->author , curr_item->title);
			goto error;
		}
		

		if(first_item != NULL)
		{
			struct item * dup_item = find_item(first_item , curr_item->f_id , curr_item->s_id);

			//Check for duplicates by seeing if we can find it in the list
			if(dup_item != NULL)
			{
				fprintf(stderr , "Item %hd.%hd (%s/%s) duplicates that for (%s/%s)\n", curr_item->f_id , curr_item->s_id , 
				curr_item->author , curr_item->title , dup_item->author , dup_item->title);
				goto error;
			}

			//If the current item is less than the first item , then we need to reassign the first item
			if(compare_item(curr_item , first_item) < 0)
			{
				curr_item->next_item = first_item;
				first_item = curr_item;
				continue;
			}

			//Find the correct place to insert the item
			struct item * insert = first_item;

			while(insert->next_item != NULL &&
				compare_item(curr_item , insert->next_item) > 0)
			{	
				insert = insert->next_item;
			}

			curr_item->next_item = insert->next_item;
			insert->next_item = curr_item;
		}
		else
			first_item = curr_item;


		continue;

		//Standard errors. Unallocate memory and continue processing
		error:
			unallocate(s_author);
			unallocate(s_title);
			unallocate(curr_item);
			continue;

		//Memory errors. Unallocate memory and exit
		memory_failure_5:
			unallocate(title);
		memory_failure_4:
			unallocate(s_author);
			goto memory_failure_2;
		memory_failure_3:
			unallocate(author);
		memory_failure_2:
			unallocate(curr_item);
		memory_failure_1:
			fprintf(stderr , "Memory allocation failed!\n");
			unallocate_items(first_item);
			fclose(item_file);
			return NULL;
	}
	
	//Item file error
	if(item_file == NULL)
		printf("Item database file error\n");
	else 
		fclose(item_file);

	return first_item;
}


int remove_whitespace(char * line)
{

	//Find the sequence of whitespace proceeding a \0
	char * start = NULL;

	int newline = 0;

	while(*line != '\0')
	{
		if(*line == '\n' || *line == ' ')
		{
			if(*line == '\n')
				newline = 1;

			if(start == NULL)
				start = line;
		}
		else
			start = NULL;

		line++;
	}


	//Replace all whitespace with \0
	while(start != NULL && *start != '\0')
	{
		*start = '\0';
		start ++;
	}

	//Return if there was a newline or not
	return newline;
}

void advance_to_newline(FILE * file)
{
	//Advance the file pointer to a newline
	while(fgetc(file) != '\n');
}
