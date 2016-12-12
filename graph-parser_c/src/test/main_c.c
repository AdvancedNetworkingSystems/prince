/*
 * main.c
 *
 *  Created on: 11 mag 2016
 *      Author: gabriel
 */

#include <stdio.h>
#include  <unistd.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
void
parse_netjson(char* buffer){
    json_object *topo = json_tokener_parse(buffer);
    if(!topo) return;
    struct json_object *o ;
    json_object_object_get_ex(topo,"links",&o);
    int size=json_object_array_length(o);
    int i;	
    for(i=0;i<size;i++){
	struct json_object *item=json_object_array_get_idx(o,i);
        struct json_object *src;
        struct json_object *tar;
        struct json_object *cost;
        json_object_object_get_ex(item,"source",&src);
        json_object_object_get_ex(item,"target",&tar);
        json_object_object_get_ex(item,"cost",&cost);
        printf ("§§> %s %s %f\n",json_object_get_string(src),json_object_get_string(tar),json_object_get_double(cost));
        
    }
}

int main(){
    char * buffer = 0;
    long length;
    FILE * f = fopen ("test/olsr-netjson.json", "rb");
    
    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length);
        if (buffer)
        {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }

    parse_netjson(buffer);
}

