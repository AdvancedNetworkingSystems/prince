/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   test.c
 * Author: principale
 *
 * Created on January 25, 2017, 5:42 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "../../prince/src/parser.h"
#include "../../prince/src/oonf.h"
#include "../../prince/src/prince.h"
#include "graph_parser.h"

char* read_file_content(char *filename)
{
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");
    
    if (handler)
    {
        // Seek the last byte of the file
        fseek(handler, 0, SEEK_END);
        // Offset from the first to the last byte, or in other words, filesize
        string_size = ftell(handler);
        // go back to the start of the file
        rewind(handler);
        
        // Allocate a string that can hold it all
        buffer = (char*) malloc(sizeof(char) * (string_size + 1) );
        
        // Read it all in one operation
        read_size = fread(buffer, sizeof(char), string_size, handler);
        
        // fread doesn't set it so put a \0 in the last position
        // and buffer is now officially a string
        buffer[string_size] = '\0';
        
        if (string_size != read_size)
        {
            // Something went wrong, throw away the memory and set
            // the buffer to NULL
            free(buffer);
            buffer = NULL;
        }
        
        // Always remember to close the file.
        fclose(handler);
    }
    
    return buffer;
}

/*
 * 
 */
int main(int argc, char** argv) {
    /*  if(argc==1)
     return 0;
     int heuristic=atoi(argv[1]);
     //to remove in case of different test
     if(argc==3)
     stop_computing_if_unchanged=atoi(argv[2])==1;
     c_graph_parser* cgp=new_graph_parser(1, heuristic);
     char * file_content=read_file_content("input.json");
     struct topology *topo=parse_netjson(file_content);
     free(file_content);
     graph_parser_parse_simplegraph(cgp,topo);
     graph_parser_calculate_bc(cgp);
     map_id_degree_bc * bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));
     graph_parser_compose_degree_bc_map(cgp,bc_degree_map);
     
     int i;
     // printf("[\n");
     printf("{\n");
     for(i=0; i<bc_degree_map->size; i++){
     //if(strcmp(bc_degree_map->map[i].id, node_name)==0){
     printf("%s:%1.50f",bc_degree_map->map[i].id,bc_degree_map->map[i].bc);
     //   break;
     //  }
     if(i<bc_degree_map->size-1)
     printf(",");
     printf("\n");
     }
     printf("}");
     delete_graph_parser(cgp);*/
    

    
    
    struct prince_handler *ph= new_prince_handler(argv[1]);
    recursive=ph->recursive;
    multithread=ph->multithreaded;
    stop_computing_if_unchanged=ph->stop_unchanged;
    ph->gp = new_graph_parser(ph->weights, ph->heuristic);
    struct graph_parser * gp_p=(struct graph_parser *)ph->gp ;
    ph->rp = new_plugin(ph->host, ph->port, ph->gp, ph->json_type);
    int iterations=2;
    do{
        //sleep(ph->refresh);
        /*if(!get_topology(ph->rp)){
         printf("Error getting topology");
         continue;
         
         }*/
        char * file_content=read_file_content("input.json");
        struct topology *t=parse_netjson(file_content);
        graph_parser_parse_simplegraph(ph->gp, t);
        destroy_topo(t);
        if(ph->rp->self_id)
            ph->self_id = strdup(ph->rp->self_id);
        clock_t start = clock();
        graph_parser_calculate_bc(ph->gp);
        clock_t end = clock();
        graph_parser_compose_degree_bc_map(ph->gp, ph->bc_degree_map);
        ph->opt_t.exec_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Calculation time: %fs\n", ph->opt_t.exec_time);
        if (!compute_timers(ph)){
            delete_prince_handler(ph);
            continue;
        }
        printf("\nId of the node we are computing is: %s\n", ph->self_id);
        if (!push_timers(ph->rp, ph->opt_t)){
            delete_prince_handler(ph);
            continue;
        }
        free(gp_p->bc);
        gp_p->bc=0;
        free_graph(&(gp_p->g));
        init_graph(&(gp_p->g));
    }while(ph->refresh && iterations-->0);
    delete_plugin(ph->rp);
    delete_prince_handler(ph);
    
    return (EXIT_SUCCESS);
}

