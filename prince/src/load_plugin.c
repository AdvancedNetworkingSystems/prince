#include "load_plugin.h"


/**
 * Load a plugin library and check if the operation is successful
 */
int load_routing_plugin(prince_handler_t result) {
	char libname[20] = "libprince_";
        char *shared_library_error;

	strcat(libname, result->proto);
	strcat(libname, ".so");

        // clear dlerror
        dlerror();
	result->plugin_handle = dlopen(libname, RTLD_LAZY);
        shared_library_error = dlerror();
	if (shared_library_error != NULL) {
                fprintf(stderr, "%s\n", shared_library_error);
                errno = ELIBACC;
	        return -1;
        }
        return 0;
}

int free_routing_plugin(prince_handler_t result) {
        char * shared_library_error;
        dlerror();
        // free routing plugin from shared library
        // with function from shared library
	delete_plugin_p(result->rp);
        // TODO: needed or symbol exists and
        // there cannot be an error?
        shared_library_error = dlerror();
        if (shared_library_error != NULL) {
                fprintf(stderr, "%s\n", shared_library_error);
                errno = ELIBACC;
                return -1;
        }
        // clear
        dlerror();
        // end TODO
	dlclose(result->plugin_handle);
        shared_library_error = dlerror();
        if (shared_library_error != NULL) {
                fprintf(stderr, "%s\n", shared_library_error);
                errno = ELIBACC;
                return -1;
        }
        return 0;
}

/** 
 * Load a symbol from the plugin library and check if the operation is successful
 */
int load_routing_plugin_symbol(prince_handler_t result, const char* symbol_name) {
        char *shared_library_error;
        // clear previous dlerror
        dlerror();
        // find the required symbol
	push_timers_p = (int (*)(routing_plugin *o, struct timers t)) dlsym(result->plugin_handle, symbol_name);
        // check if there has been any error when
        // resolving the symbol
        shared_library_error = dlerror();
        if (shared_library_error != NULL) {
                fprintf(stderr, "%s\n", shared_library_error);
                errno = ELIBACC;
                return -1;
        }
        return 0;
}
