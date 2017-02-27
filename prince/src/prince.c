/*
 Copyright (c) 2016 Gabriele Gemmi <gabriel@autistici.org>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */
/*
 * prince.c
 *
 *  Created on: 12 mag 2016
 *      Author: gabriel
 */
#include "prince.h"
#include <unistd.h>
#include <time.h>
/*TODO: remove*/

char* read_file_content(char *filename)
{
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");
    
    if (handler)
    {
        
        fseek(handler, 0, SEEK_END);
        string_size = ftell(handler);
        rewind(handler);
        
        buffer = (char*) malloc(sizeof(char) * (string_size + 1) );
        
        read_size = fread(buffer, sizeof(char), string_size, handler);
        
        buffer[string_size] = '\0';
        
        if (string_size != read_size)
        {
            free(buffer);
            buffer = NULL;
        }
        
        fclose(handler);
    }
    
    return buffer;
}



routing_plugin* (*new_plugin_p)(char* host, int port, c_graph_parser *gp, int json_type);
int (*get_topology_p)(routing_plugin *o);
int (*push_timers_p)(routing_plugin *o, struct timers t);
void (*delete_plugin_p)(routing_plugin* o);

/**
 * Main routine of Prince. Collect topology, parse it, calculate bc and timers, push them back.
 * @param argv[1] <- config filename
 * @return 1 on success, 0 on error
 */
int
main(int argc, char* argv[]){
    if(argc<2){
        printf("No conf file specified. Exiting.\n");
        return -1;
    }
    struct prince_handler *ph= new_prince_handler(argv[1]);
    if(ph==0)
        return -1;
    
#ifndef unique
    ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));
    /*cycle each 'refresh' seconds*/
    do{
        sleep(ph->refresh);
        ph->gp = new_graph_parser(ph->weights, ph->heuristic?1:0);
        ph->rp = new_plugin_p(ph->host, ph->port, ph->gp, ph->json_type);
        if(!get_topology_p(ph->rp)){
            printf("Error getting topology");
            continue;
            
        }
        if(ph->rp->self_id)
            ph->self_id = strdup(ph->rp->self_id);
        clock_t start = clock();
        graph_parser_calculate_bc(ph->gp);
        clock_t end = clock();
        graph_parser_compose_degree_bc_map(ph->gp, ph->bc_degree_map);
        ph->opt_t.exec_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Calculation time: %fs\n", ph->opt_t.exec_time);
        printf("==");
        if (!compute_timers(ph)){
            delete_prince_handler(ph);
            continue;
        }
        printf("==");
        printf("\nId of the node we are computing is: %s\n", ph->self_id);
        if (!push_timers_p(ph->rp, ph->opt_t)){
            delete_prince_handler(ph);
            continue;
        }
        delete_plugin_p(ph->rp);
    }while(ph->refresh);
    delete_prince_handler(ph);
#else
    recursive=ph->recursive;
    multithread=ph->multithreaded;
    stop_computing_if_unchanged=ph->stop_unchanged;
    ph->gp = new_graph_parser(ph->weights, ph->heuristic);
    struct graph_parser * gp_p=(struct graph_parser *)ph->gp ;
    ph->rp = new_plugin_p(ph->host, ph->port, ph->gp, ph->json_type);
    do{
        sleep(ph->refresh);
        if(!get_topology_p(ph->rp)){
            printf("Error getting topology");
            continue;
            
        }
        if(ph->rp->self_id){
            if(ph->self_id!=0)
                free(ph->self_id);
            ph->self_id = strdup(ph->rp->self_id);
        }
        clock_t start = clock();
        graph_parser_calculate_bc(ph->gp);
        clock_t end = clock();
        ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));
        ph->bc_degree_map->size=gp_p->g.nodes.size;
        ph->bc_degree_map->map=0;
        graph_parser_compose_degree_bc_map(ph->gp, ph->bc_degree_map);
        ph->opt_t.exec_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("\nCalculation time: %fs\n", ph->opt_t.exec_time);
        if (!compute_timers(ph)){
            delete_prince_handler(ph);
            continue;
        }
        printf("Id of the node we are computing is: %s\n", ph->self_id);
        if (!push_timers_p(ph->rp, ph->opt_t)){
            delete_prince_handler(ph);
            continue;
        }
        free(gp_p->bc);
        gp_p->bc=0;
        bc_degree_map_delete(ph->bc_degree_map);
        free_graph(&(gp_p->g));
        init_graph(&(gp_p->g));
    }while(ph->refresh);
    delete_prince_handler(ph);
#endif	
    return 0;
}

/**
 * Initalize a new prince handler
 * @param host host address as a string
 * @return pointer to prince handler
 */
struct prince_handler*
new_prince_handler(char * conf_file){
    struct prince_handler* ph = (struct prince_handler*) malloc(sizeof(struct prince_handler));
    ph->def_t.h_timer=2.0;
    ph->def_t.tc_timer=5.0;
    /* ph->bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));*/
    /*setting to undefined all params*/
    ph->proto=-1;
    ph->host=0;
    ph->port=-1;
    ph->refresh=-1;
    ph->self_id=0;
    if(read_config_file(ph, conf_file)==0)
        return 0;
    switch(ph->proto){
        case 0: /*olsr*/
            ph->json_type=0;
#ifndef unique
            ph->plugin_handle = dlopen ("libprince_olsr.so", RTLD_LAZY);
#else
	    ph->plugin_handle = dlopen ("libprince_olsr_c.so", RTLD_LAZY);
#endif
            break;
        case 1: /*oonf*/
#ifndef unique
            ph->plugin_handle = dlopen ("libprince_oonf.so", RTLD_LAZY);
#else
	    ph->plugin_handle = dlopen ("libprince_oonf_c.so", RTLD_LAZY);
#endif
            break;
    }
    if(!ph->plugin_handle)
        return 0;
    
    new_plugin_p = (routing_plugin* (*)(char* host, int port, c_graph_parser *gp, int json_type)) dlsym(ph->plugin_handle, "new_plugin");
    get_topology_p = (int (*)(routing_plugin *o)) dlsym(ph->plugin_handle, "get_topology");
    push_timers_p = (int (*)(routing_plugin *o, struct timers t)) dlsym(ph->plugin_handle, "push_timers");
    delete_plugin_p = (void (*)(routing_plugin *o)) dlsym(ph->plugin_handle, "delete_plugin");
    return ph;
}
/**
 * Delete a Prince handler and free all the memory
 * @param struct prince_handler* pointer to the prince_handler struct.
 */
void delete_prince_handler(struct prince_handler* ph){
    delete_plugin_p(ph->rp);
    dlclose(ph->plugin_handle);
    char* tmp=dlerror();
    free(tmp);
    /*bc_degree_map_delete(ph->bc_degree_map);*/
    free(ph->self_id);
    free(ph->host);
    free(ph);
}


/**
 * Compute the constants needded for the timer calculation
 * and store them in ph->c
 * @param pointer to the prince_handler object
 * @return 1 if success, 0 if fail
 */
int
compute_constants(struct prince_handler *ph){
    map_id_degree_bc *m_degree_bc = ph->bc_degree_map;
    struct timers t = ph->def_t;
    int degrees=0, i;
    for(i=0; i<m_degree_bc->size;i++){
        degrees+=m_degree_bc->map[i].degree;
        /*printf("%s %f\n", m_degree_bc->map[i].id, m_degree_bc->map[i].bc);*/
    }
    ph->c.R = m_degree_bc->n_edges;
    ph->c.O_H = degrees/t.h_timer;
    ph->c.O_TC = m_degree_bc->size*ph->c.R/t.tc_timer;
    double sqrt_sum1=0, sqrt_sum2=0;
    for(i=0; i<m_degree_bc->size; i++){
        sqrt_sum1+=sqrt(m_degree_bc->map[i].degree * m_degree_bc->map[i].bc);
        sqrt_sum2+=sqrt(ph->c.R*m_degree_bc->map[i].bc);
    }
    ph->c.sq_lambda_H = sqrt_sum1/ph->c.O_H;
    ph->c.sq_lambda_TC = sqrt_sum2/ph->c.O_TC;
    return 1;
}

/**
 * Compute the timers
 * and store them in ph->opt_t
 * @param pointer to the prince_handler object
 * @return 1 if success, 0 if fail
 */
int
compute_timers(struct prince_handler *ph){
    compute_constants(ph);
    int my_index=-1, i;
    for(i=0; i<ph->bc_degree_map->size; i++){
        if(strcmp(ph->bc_degree_map->map[i].id, ph->self_id)==0){
            my_index=i;
        }
    }
    if(my_index==-1) return 0;
    ph->opt_t.h_timer = sqrt(ph->bc_degree_map->map[my_index].degree / ph->bc_degree_map->map[my_index].bc) * ph->c.sq_lambda_H;
    ph->opt_t.tc_timer = sqrt(ph->c.R/ph->bc_degree_map->map[my_index].bc)*ph->c.sq_lambda_TC;
    return 1;
}

/**
 * Given a file, return its extension
 * 
 * @param filename A file name
 * @return The file extension
 */
const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

/**
 * Given a json in the following format:
 * {
 *    "proto":{  
 *       "protocol":"oonf",
 *       "host":"127.0.0.1",
 *       "port":2019,
 *       "refresh":1
 *    },
 *    "graph-parser":{  
 *       "heuristic":1,
 *       "weights":0,
 *       "recursive":1,
 *       "stop_unchanged":0,
 *       "multithreaded":1
 *    }
 * }
 * it parses it and initializes the program parameter
 * @param filepath A json file path
 * @param ph The prince_handler, for saving the configuration
 * @return Whether the reading ended successfully 
 */
bool parse_json_config(char *filepath,struct prince_handler *ph){
    char * buffer = read_file_content(filepath);
    int completed=0;
    bool heuristic_set=false,weights_set=false,
    recursive_set=false,stop_unchanged_set=false,multithreaded_set=false;
    struct json_object *jobj = json_tokener_parse(buffer);
    json_object_object_foreach(jobj, key, val) {
        if(strcmp(key, "proto")==0){
            if(json_object_get_type(val)==json_type_object){
                json_object_object_foreach(val, key_i, val_i) {
                    if(ph->proto<0&&strcmp(key_i, "protocol")==0){
                        if(json_object_get_type(val_i)==json_type_string){
                            const char * content=json_object_get_string(val_i);
			    if(strcmp(content, "oslr")==0){
                                ph->proto=0;
                                completed++;
                            }else if(strcmp(content, "oonf")==0){
                                ph->proto=1;
                                completed++;
                            }
                        }
                    }else if(ph->host==0&&strcmp(key_i, "host")==0){ 
                        if(json_object_get_type(val_i)==json_type_string){
                            const char * content=json_object_get_string(val_i);
                            ph->host=strdup(content);
                            completed++;
                            
                        }
                    }else if(ph->port<0&&strcmp(key_i, "port")==0){ 
                        if(json_object_get_type(val_i)==json_type_int){
			    ph->port=json_object_get_int(val_i);
                            completed++;
                        }
                    }else if(strcmp(key_i, "refresh")==0){ 
                        if(ph->refresh<0&&json_object_get_type(val_i)==json_type_int){
                            ph->refresh=json_object_get_int(val_i);
                            completed++;
                        }
                    }
                }
            }
        }else if(strcmp(key, "graph-parser")==0){
            if(json_object_get_type(val)==json_type_object){
                json_object_object_foreach(val, key_i, val_i) {
                    if(!heuristic_set&&strcmp(key_i, "heuristic")==0){
                        if(json_object_get_type(val_i)==json_type_int){
                            ph->heuristic=json_object_get_int(val_i)==1;
                            heuristic_set=true;
                        }
                    }else if(strcmp(key_i, "weights")==0){ 
                        if(!weights_set&&json_object_get_type(val_i)==json_type_int){
                            ph->weights=json_object_get_int(val_i)==1;
                            weights_set=true;
                        }
                    }else if(strcmp(key_i, "recursive")==0){ 
                        if(!recursive_set&&json_object_get_type(val_i)==json_type_int){
                            ph->recursive=json_object_get_int(val_i)==1;
                            recursive_set=true;
                        }
                    }else if(strcmp(key_i, "stop_unchanged")==0){ 
                        if(!stop_unchanged_set&&json_object_get_type(val_i)==json_type_int){
                            ph->stop_unchanged=json_object_get_int(val_i);
                            stop_unchanged_set=true;
                        }
                    }else if(strcmp(key_i, "multithreaded")==0){ 
                        if(!multithreaded_set&&json_object_get_type(val_i)==json_type_int){
                            ph->multithreaded=json_object_get_int(val_i);
                            multithreaded_set=true;
                        }
                    }
                }
            }
        }
    }
    if(jobj!=0){
        json_object_put(jobj);
        /* json_object_object_del(jobj, "");*/
    }
    free(buffer);
    if(completed==4&&heuristic_set && weights_set && recursive_set 
            && stop_unchanged_set && multithreaded_set)
        return true;
    return false;
    
}

/**
 * Read the ini configuration and populate struct prince_handler
 * @param *ph pinter to the prince_handler object
 * @param *filepath path to the configuration file
 * @return 1 if success, 0 if fail
 */
int
read_config_file(struct prince_handler *ph, char *filepath){
    if(!((strcmp(get_filename_ext(filepath),"json")==0 && parse_json_config(filepath, ph)) ||(ini_parse(filepath, handler, ph)))) {
        printf("Cannot read configuration file '%s' (Either format or content not compliant or complete). Exiting.\n", filepath);
        return 0;
    }
    
    printf("Config loaded from %s\n", filepath);
    return 1;
}
