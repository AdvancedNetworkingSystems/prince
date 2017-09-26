/*
 * File:   algorithms.h
 * Author: mb03
 *
 * Created on December 12, 2016, 5:55 PM
 */

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <pthread.h>
#include "biconnected.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * This is the algorithm described in the linked papers.
     * Given a network, in the form of a weighted graph, it computes the
     * centrality of each node based on the amount of routes that are based on it.
     * http://www.algo.uni-konstanz.de/publications/b-fabc-01.pdf
     * http://algo.uni-konstanz.de/publications/b-vspbc-08.pdf
     */
    double * betweeness_brandes(struct graph *g,
                                bool         endpoints,
                                int *        articulation_point_val);

    /**
     * This is the algorithm described in the linked paper.
     * It reaches the exact results of the previous one, but it is implemented
     * using a divide et impera mechanism based on biconnected components in order
     * to speed up computation.
     * http://algo.uni-konstanz.de/publications/pzedb-hsbcc-12.pdf
     */
    double * betwenness_heuristic(struct graph *g,
                                  bool         recursive);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_H */

