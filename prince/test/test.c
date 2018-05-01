#include <CUnit/Basic.h>
#include "../include/config_graph.h"
#include "../include/common.h"
#include "blob.h"

void config_graph_test(void)
{
	char *filepath = "./test.conf";
	FILE *fp;
	fp = fopen(filepath, "w");
	CU_PASS();

	if (fp == NULL) {
		CU_FAIL("Cannot open file to write configuration");
	}
	fprintf(fp, "%s\n", conf_file);
	fclose(fp);
	// load_graph_config(filepath, gconf);
	// load_proto_config(filepath, pconf);
	// CU_ASSERT(gconf->heuristic == 1);
	// CU_ASSERT(gconf->multithreaded == 0);
	// CU_ASSERT(gconf->recursive == 0);
	// CU_ASSERT(gconf->stop_unchanged == 0);
	// CU_ASSERT(gconf->weights == 1);
	// CU_ASSERT(strcmp(pconf->json_type, "jsoninfo") == 0);
	// CU_ASSERT(pconf->host == "127.0.0.1");
	// CU_ASSERT(pconf->proto == "olsr");
	// CU_ASSERT(pconf->port == 2009);
	// CU_ASSERT(pconf->timer_port == 2008);
	// CU_ASSERT(pconf->refresh == 1);
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
