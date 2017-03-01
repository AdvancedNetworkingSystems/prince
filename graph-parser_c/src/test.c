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

int main(int argc, char** argv) {
    if(argc==1)
        return 0;
    int heuristic=atoi(argv[1]);
    //to remove in case of different test
    multithread=false;
    if(argc==3){
        stop_computing_if_unchanged=atoi(argv[2])==1;
    }
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
    delete_graph_parser(cgp);
    return (EXIT_SUCCESS);
}

 */