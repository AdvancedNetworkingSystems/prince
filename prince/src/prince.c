#include "prince.h"

/**
* Main routine of Prince. Collect topology, parse it, calculate bc and timers, push them back.
* @param argv[1] <- config filename
* @return 0 on success
*/
int main(int argc, char* argv[]) {
	FILE *log = NULL;
	if (argc < 2) {
		fprintf(stderr, "No conf file specified. Exiting.\n");
               exit(1);
	}
	fprintf(stdout, "Prince Started\n");

	prince_handler_t ph = new_prince_handler(argv[1]);
        if (ph == INVALID_PRINCE_HANDLER) {
                fprintf(stderr, "Could not create prince handler\n");
                exit(EXIT_FAILURE);
        }

        //if (load_proto_config(argv[1], ph->proto_config)) {
        //        fprintf(stderr, "Could not load section 'proto' from config\n");
        //}
        //if (load_graph_config(argv[1], ph->graph_config)) {
        //        fprintf(stderr, "Could not load section 'graph' from config\n");
        //}

	if (ph->log_file) {
		log = fopen(ph->log_file, "a+");
		if (log == NULL) {
			printf("Could not open log file <%s>; Continuing without\n", ph->log_file);
			free(ph->log_file);
			ph->log_file = NULL;
		} else {
			printf("Logging to <%s>\n", ph->log_file);
                }
	}

	recursive                   = ph->recursive;
	multithread                 = ph->multithreaded;
	stop_computing_if_unchanged = ph->stop_unchanged;

	signal(SIGPIPE, signal_callback_handler);

	struct graph_parser * gp_p = (struct graph_parser *) ph->gp;

	do {
		sleep(ph->refresh);
	} while (get_initial_timers_p(ph->rp, &ph->def_t));


	int go = 1;
	do {
		sleep(ph->refresh);
		if (get_topology_p(ph->rp)) {
			fprintf(stderr, "Error getting topology\n");
			sleep(ph->sleep_onfail);
			continue;
		}

                if (valid_topo(ph->rp->t)) {
                        fprintf(stderr, "Invalid topology data\n");
                        exit(EXIT_FAILURE);
                }

                update_prince_id(ph);
		graph_parser_parse_simplegraph(ph->gp, ph->rp->t);
		free_topo(ph->rp->t);

		clock_t start = clock();
		graph_parser_calculate_bc(ph->gp);
		clock_t end = clock();
		ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));
                if (ph->bc_degree_map == NULL) {
                        perror("prince");
                        exit(EXIT_FAILURE);
                }
		ph->bc_degree_map->size = gp_p->g.nodes.size;
		ph->bc_degree_map->map = 0;
		graph_parser_compose_degree_bc_map(ph->gp, ph->bc_degree_map);
		ph->opt_t.exec_time = (double) (end - start) / CLOCKS_PER_SEC;
		ph->opt_t.centrality = get_self_bc(ph);

		if (log) {
			log = fopen(ph->log_file, "a+");
			struct timeval tv;
			gettimeofday(&tv, NULL);
			fprintf(log, "%i\t%4.4f\t%4.4f\t%4.4f\t%4.4f\n", tv.tv_sec, ph->opt_t.tc_timer, ph->opt_t.h_timer, ph->opt_t.exec_time, ph->opt_t.centrality);
			fclose(log);
		}
		if (compute_timers(ph)) {
			printf("Can't find myself in topology! Help i'm lost\n");
			sleep(ph->sleep_onfail);
			continue;
		}
		if (!push_timers_p(ph->rp, ph->opt_t)) {
			sleep(ph->sleep_onfail);
			continue;
		}
		free(gp_p->bc);
		gp_p->bc = 0;
		free_bc_degree_map(ph->bc_degree_map);
		free_graph(&(gp_p->g));
		init_graph(&(gp_p->g));
		fflush(stdout);
	} while (go);

	if (free_prince_handler(ph)) {
                perror("prince");
        }
	fprintf(stdout, "Prince Exited\n");
	if (log) {
		log = fopen(ph->log_file, "a+");
		fprintf(log, "Prince Exited\n");
		fclose(log);
	}
	return 0;
}


/**
* Compute the constants needded for the timer calculation
* and store them in ph->c
* @param pointer to the prince_handler object
* @return 0 if success, -1 if fail
*/
int compute_constants(prince_handler_t ph) {
	map_id_degree_bc *m_degree_bc = ph->bc_degree_map;
	struct timers t = ph->def_t;
	int degrees = 0, i;
	for (i = 0; i < m_degree_bc->size; i++) {
		degrees += m_degree_bc->map[i].degree;
		/*printf("%s %f\n", m_degree_bc->map[i].id, m_degree_bc->map[i].bc);*/
	}
	ph->c.R    = m_degree_bc->n_edges;
	ph->c.O_H  = degrees / t.h_timer;
	ph->c.O_TC = m_degree_bc->size * ph->c.R / t.tc_timer;
	double sqrt_sum1 = 0, sqrt_sum2 = 0;
	for (i = 0; i < m_degree_bc->size; i++) {
		sqrt_sum1 += sqrt(m_degree_bc->map[i].degree * m_degree_bc->map[i].bc);
		sqrt_sum2 += sqrt(ph->c.R*m_degree_bc->map[i].bc);
	}
	ph->c.sq_lambda_H  = sqrt_sum1 / ph->c.O_H;
	ph->c.sq_lambda_TC = sqrt_sum2 / ph->c.O_TC;
	return 0;
}

/**
* Compute the timers
* and store them in ph->opt_t
* @param pointer to the prince_handler object
* @return 0 if success, -1 if fail
*/
int compute_timers(prince_handler_t ph) {
	int t = compute_constants(ph);
        if (t == -1) {
                fprintf(stderr, "Could not compute constants\n");
                exit(EXIT_FAILURE);
        }
	int my_index = -1, i;
	for (i = 0; i< ph->bc_degree_map->size; i++) {
		if (strcmp(ph->bc_degree_map->map[i].id, ph->self_id) == 0) {
			my_index = i;
		}
	}
	if (my_index == -1) {
                return -1;
        }
	ph->opt_t.h_timer = sqrt(ph->bc_degree_map->map[my_index].degree / ph->bc_degree_map->map[my_index].bc) * ph->c.sq_lambda_H;
	ph->opt_t.tc_timer = sqrt(ph->c.R/ph->bc_degree_map->map[my_index].bc) * ph->c.sq_lambda_TC;
	return 0;
}

double get_self_bc(prince_handler_t ph) {
	map_id_degree_bc *m_degree_bc = ph->bc_degree_map;
	int i;
	for(i = 0; i < m_degree_bc->size; i++) {
		if (strcmp(ph->self_id, m_degree_bc->map[i].id) == 0) {
			return m_degree_bc->map[i].bc;
                }
	}
	return 0;
}

void signal_callback_handler(int signum){
        printf("Caught signal SIGPIPE %d\n",signum);
}
