/*
 * This file is part of EGL/GLES2.0 Extension Test for API TRACE
 * Using simple rendering senario, it calls all new defined function and tokens of extension. 
 */

#include "common.h"

#define NUM_OF_TEST_CASE 17

int main(int argc, char** argv )
{
	int test_all = 0;
	int test_case = 0;
	int i;
	Test_List* test_list[NUM_OF_TEST_CASE];
	init_test_case(test_list);

	/* CHOOSE THE TEST CASE */
	char *input = argv[1];
	if(input == NULL || *input == 'h')
	{
		fprintf(stderr, "- PUT TEST CASE NUMBER OR \n");
		fprintf(stderr, "'a' -> RUN ALL TEST CASE\n");
		fprintf(stderr, "'l' -> SEE THE TEST CASE LIST\n");
		goto finish;
	}
	if(*input == 'a')
	{
		test_all = 1; 
	}
	
	/* PRINT ALL TSET CASE LIST */
	else if(*input == 'l')
	{
		print_test_case(test_list);
		goto finish;
	}

	if(test_all)
	{
		i = 0;
		fprintf(stderr, "- START TEST ALL CASES !! \n");
		while(test_list[i]->test_function != NULL)
		{
			test_list[i]->test_function();
			i++;
		}
	}
	else
	{
		test_case = atoi(input);
		/* RUN ONE TEST CASE */
		if(test_case > NUM_OF_TEST_CASE-1 || test_case <=0)
		{
			fprintf(stderr, "TOTAL TEST CASE IS %d, PLEASE PUT %d ~ %d \n", NUM_OF_TEST_CASE-1, 1, NUM_OF_TEST_CASE-1);
			goto finish;
		}
		if(test_list[test_case-1]->test_function != NULL)
			test_list[test_case-1]->test_function();
	}
finish :
	return 1;
}


 

