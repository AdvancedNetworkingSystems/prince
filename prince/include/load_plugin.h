#ifndef LOAD_PLUGIN_H_
#define LOAD_PLUGIN_H_
#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include "prince_handler.h"

int load_routing_plugin(prince_handler_t result);
int load_routing_plugin_symbol(prince_handler_t result, const char* symbol_name);
#endif /* LOAD_PLUGIN_H_ */
