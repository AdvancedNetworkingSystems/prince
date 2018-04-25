#include <CUnit/Basic.h>
#include "../include/config_graph.h"

void config_graph_test(void)
{
	char *filepath = "./test.conf";
	graph_config_t gconf = new_graph_config();
	// load_graph_config(filepath, gconf);
	CU_ASSERT(1);
}

int main(int argc, char **argv)
{

	CU_pSuite pSuite = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* add a suite to the registry */
	pSuite = CU_add_suite("Suite_1", NULL, NULL);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* add the tests to the suite */
	if (NULL
	    == CU_add_test(pSuite, "config_graph test 1", config_graph_test)) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
