#include "config_proto.h"

#include "prince.h"

int load_proto_config(const char *filepath, proto_config_t conf)
{
	if (conf == INVALID_PROTO_CONFIG) {
		return -1;
	}
	prince_handler_t temp = new_prince_handler(filepath);
	if (temp == INVALID_PRINCE_HANDLER) {
		return -1;
	}
	conf->host = temp->host;
	conf->json_type = temp->json_type;
	conf->port = temp->port;
	conf->proto = temp->proto;
	conf->refresh = temp->refresh;
	conf->sleep_onfail = temp->sleep_onfail;
	conf->timer_port = temp->timer_port;
	free_prince_handler(temp);
	return 0;
}

proto_config_t new_proto_config(void)
{
	proto_config_t result = (proto_config_t)malloc(PROTO_CONFIG_SIZE);
	if (result == INVALID_PROTO_CONFIG) {
		perror(NULL);
		return INVALID_PROTO_CONFIG;
	}
	memset(result, 0, PROTO_CONFIG_SIZE);
	result->port = -1;
	result->refresh = -1;
	result->sleep_onfail = -1;
	return result;
}

void free_proto_config(proto_config_t conf)
{
	free(conf->proto);
	free(conf);
}
