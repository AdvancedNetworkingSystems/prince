/*
 * File:   network_change.h
 * Author: mb03
 *
 * Created on January 28, 2017, 11:48 AM
 */

#ifndef NETWORK_CHANGE_H
#define NETWORK_CHANGE_H

#include "brandes.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * These functions aims to infer if the network is changed, and if it is so,
     * they measure this modification. If it is lower than a predefined
     * threshold, they return the old values.
     *
     */

    /**
     * !!! NOTE !!!
     * This is the function that decides whether to recompute.
     * Further details in network_change.c.
     * If you want to change this aspect, you have to replace this function.
     *
     */
    bool recompute(double deviation,
                   double deviation_old,
                   int    biconnected_num,
                   int    old_biconnected_num);

    double * is_network_changed(struct list *biconnected_components_subgraph,
                                int         node_num,
                                int *       biconnected_num,
                                float *     standard_deviation_bic,
                                float *     standard_deviation_edge,
                                int *       result_size,
                                char ***    node_names);

    void write_file(int         connected_comp_num,
                    float       standard_deviation_bic,
                    float       standard_deviation_edge,
                    int         size,
                    double *    ret_vals,
                    struct list *list_of_nodes);

    void copy_old_values(double *    old_vals,
                         double *    vals,
                         char **     names,
                         int         names_count,
                         struct list *list_of_nodes);


#ifdef __cplusplus
}
#endif

#endif /* NETWORK_CHANGE_H */
