
#include "network_change.h"
#include <json-c/json_object.h>

/**
 * This function tells whether we recompute the graph based on some parameters.
 * In order to improve the speed, i.e. to reduce the number the algorithm
 * completes, instead of using already computed value, you have to change this
 * function.
 *
 * @param deviation_bic The standard deviation of the biconnected components
 * sizes in current graph
 * @param old_deviation_bic The standard deviation of the biconnected components
 * sizes of the last computed graph
 * @param biconnected_num The number of biconnected component of the
 * current graph
 * @param old_biconnected_num The number of biconnected component of the last
 * computed graph
 * @param old_node_num The number of nodes in the last computed graph
 * @param node_num The number of nodes in the present graph
 * @param deviation_edge The standard deviation of the current graph edges
 * weights
 * @param old_deviation_edge The standard deviation of the last computed
 * graph edges weights
 *
 * @return whether we have to recompute the graph
 */
bool is_to_recompute(double deviation_bic, double old_deviation_bic,
                     int biconnected_num, int old_biconnected_num, int node_num,
                     int old_node_num, double deviation_edge,
                     double old_deviation_edge)
{
        float stdd = 1;

        if (old_deviation_bic > 0) {
                stdd =
                    abs(deviation_bic - old_deviation_bic) / old_deviation_bic;
        }

        int nn = abs(biconnected_num - old_biconnected_num);
        float stde = 1;

        if (old_deviation_edge > 0) {
                stde = abs(deviation_edge - old_deviation_edge) /
                       old_deviation_edge;
        }

        int nb = abs(node_num - old_node_num);

        // parameters are chosen from test on real and simulated networks, see
        // py scripts for details
        return (stdd >= 0.05) && (nn >= 4) && (stde >= 0.005) && (nb >= 0);
}

/**
 * Write to a predefined file, network.dat, a series of information.
 * If network is not changed, these info will return as result and the
 * computation will stop.
 *
 *
 * @param connected_comp_num the number of biconnected component
 * @param standard_deviation_bic the standard deviation
 * of the size of biconnected component
 * @param standard_deviation_edge the standard deviation
 * of edge weights
 * @param size the size of the result (could be 0)
 * @param ret_vals  a pointer values of the result (could be 0, null pointing)
 * @param list_of_nodes list of nodes in the graph, used for storing id
 */
void write_file(int connected_comp_num, float standard_deviation_bic,
                float standard_deviation_edge, int size, double *ret_vals,
                struct list *list_of_nodes)
{
        FILE *fp;

        fp = fopen("network.dat", "w+");

        if (fp == NULL) {
                printf("I couldn't open results.dat for writing.\n");
                exit(0);
        }

        struct node_list *nl;

        int str_len = 0;

        for (nl = list_of_nodes->head; nl != 0; nl = nl->next) {
                struct node_graph *ng = (struct node_graph *)nl->content;

                int str_len_tmp = strlen(ng->name);

                str_len = ((str_len > str_len_tmp) ? str_len : str_len_tmp);
        }

        fprintf(fp, "%d, %f, %f, %d\n", connected_comp_num,
                (float)standard_deviation_bic, (float)standard_deviation_edge,
                list_of_nodes->size);
        fprintf(fp, "%d, %d\n", size, str_len + 1);

        int i = 0;

        for (nl = list_of_nodes->head; nl != 0; nl = nl->next) {
                struct node_graph *ng = (struct node_graph *)nl->content;

                fprintf(fp, "%f,%s\n", (float)ret_vals[ng->node_graph_id],
                        ng->name);
        }

        fclose(fp);
}

;

/**
 * Read from a predefined file, network.dat, a series of information.
 * The values are returned as content of passed pointer parameters.
 * If everything was fine, the boolean true is returned.
 *
 * @param connected_comp_num An int pointer, will store the last number of
 * biconnected component, if any
 * @param standard_deviation_bic A float pointer, will store the last value of
 * standard deviation of biconnected size, if any
 * @param standard_deviation_edge A float pointer, will store the last value of
 * standard deviation of edge weight, if any
 * @param node_num A int pointer, will store the last number of nodes , if any
 * @param size An int pointer, will store the size of last result, if any.
 * @param ret_vals  A double pointer, will store the last result values, if any.
 * @param node_names Pointer to char * array, will retrieve the list of names
 * @return Whether everything was fine
 */
bool read_file(int *connected_comp_num, float *standard_deviation_bic,
               float *standard_deviation_edge, int *node_num, int *size,
               double **ret_vals, char ***node_names)
{
        FILE *fp;

        fp = fopen("network.dat", "r");

        if (fp == NULL) {
                return false;
        }

        bool ret_val = (fscanf(fp, "%d, %f, %f, %d\n", connected_comp_num,
                               standard_deviation_bic, standard_deviation_edge,
                               node_num) == 4);
        int string_max_len = -1;

        ret_val =
            ret_val && (fscanf(fp, "%d, %d\n", size, &string_max_len) == 2);

        char *buffer = malloc(sizeof(char) * string_max_len);
        int i = 0;

        (*ret_vals) = malloc(sizeof(double) * (*size));
        (*node_names) = malloc(sizeof(char *) * (*size));

        for (; ret_val && (i < *size); i++) {
                float tmp;

                ret_val = ret_val && (fscanf(fp, "%f,%s\n", &tmp, buffer) == 2);
                (*node_names)[i] = strdup(buffer);
                (*ret_vals)[i] = (double)tmp;
        }

        fclose(fp);
        free(buffer);

        return ret_val;
}

;

/**
 * Computes and returns in the given parameters the number of connected
 * components and the standard deviation of their sizes.
 *
 * @param biconnected_components_subgraph The list of biconnected components,
 * divided among all connected subgraphs.
 * @param connected_num An int pointer, will store the last number of
 * biconnected component.
 * @param standard_deviation_bic A float pointer, will store the last value of
 * standard deviation of biconnected components size.
 */
void compute_mean_number(struct list *biconnected_components_subgraph,
                         int *connected_num, float *standard_deviation)
{
        struct node_list *nl;

        float mean = 0;
        int biconnected_comp_count = 0;

        for (nl = biconnected_components_subgraph->head; nl != 0;
             nl = nl->next) {
                struct list *tmp = (struct list *)nl->content;

                struct node_list *nl2 = tmp->head;

                for (; nl2 != 0; nl2 = nl2->next) {
                        struct connected_component *cc =
                            (struct connected_component *)nl2->content;

                        biconnected_comp_count++;

                        mean += cc->g.nodes.size;
                }
        }

        mean /= biconnected_comp_count;

        float std = 0;

        for (nl = biconnected_components_subgraph->head; nl != 0;
             nl = nl->next) {
                struct list *tmp = (struct list *)nl->content;

                struct node_list *nl2 = tmp->head;

                for (; nl2 != 0; nl2 = nl2->next) {
                        struct connected_component *cc =
                            (struct connected_component *)nl2->content;

                        std += pow((((float)cc->g.nodes.size) - mean), 2);
                }
        }

        std /= biconnected_comp_count;
        (*connected_num) = biconnected_comp_count;
        (*standard_deviation) = std;
}

/**
 * This function returns the result of last computation if the network is
 * changed, otherwise 0. In this sense detects if the network is changed.
 * This change is based on the number of biconnected component and the standard
 * deviations of biconnected component sizes. The last value is the most
 * important.
 * We use it, as a reference value, to detect if the network is changed.
 *
 * @param biconnected_components_subgraph The list of biconnected components,
 * divided among all connected subgraphs.
 * @param node_num number of nodes in the current graph
 * @param biconnected_num An int pointer, will store the number of
 * biconnected component
 * @param standard_deviation_bic A float pointer, will store the standard
 * deviation of biconnected sizes
 * @param standard_deviation_edge A float pointer,
 * will store the standard deviation of edges weights
 * @param result_size A int pointer,  will store the number of last results
 * @param node_names Pointer to char * array, will retrieve the list of names
 * @return the last results if the network is not changed, o.w. a zero pointer.
 */
double *is_network_changed(struct list *biconnected_components_subgraph,
                           int node_num, int *biconnected_num,
                           float *standard_deviation_bic,
                           float *standard_deviation_edge, int *result_size,
                           char ***node_names)
{
        int biconnected_num_old = -1;
        float standard_deviation_bic_old = -1;
        int node_num_old = -1;
        float standard_deviation_edge_old = -1;

        compute_mean_number(biconnected_components_subgraph, biconnected_num,
                            standard_deviation_bic);

        double *ret_val;

        if (!read_file(&biconnected_num_old, &standard_deviation_bic_old,
                       &standard_deviation_edge_old, &node_num_old, result_size,
                       &ret_val, node_names)) {
                return 0;
        }

        bool recompute = is_to_recompute(
            *standard_deviation_bic, standard_deviation_bic_old,
            *biconnected_num, biconnected_num_old, node_num, node_num_old,
            *standard_deviation_edge, standard_deviation_edge_old);

        if (recompute) {
                free(ret_val);

                return 0;
        } else {
                return ret_val;
        }
}

void copy_old_values(double *old_vals, double *vals, char **names,
                     int names_count, struct list *list_of_nodes)
{
        int i;

        struct node_list *nl;

        struct node_graph *ng = 0;

        for (i = 0; i < names_count; i++) {
                for (nl = list_of_nodes->head; nl != 0; nl = nl->next) {
                        ng = (struct node_graph *)nl->content;

                        if (strcmp(ng->name, names[i]) == 0) {
                                break;
                        }
                }

                if (ng != 0) {
                        vals[ng->node_graph_id] = old_vals[i];
                }

                ng = 0;
        }
}
