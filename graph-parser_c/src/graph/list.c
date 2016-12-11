/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "list.h"

void init_list( struct list * q){
    q->head=0;
    q->tail=0;
    q->size=0;
}
void enqueue_list(struct list * q,void * item){
    struct  node_list * n=(struct node_list*)malloc(sizeof(struct node_list));
    n->content=item;
    if(q->head==0){
        n->prev=0;
        n->next=0;
        q->head=n;
        q->size=1;
        q->tail=n;
    }else{
        n->prev=q->tail;
        q->tail->next=n;
        n->next=0;
        q->tail=n;
        q->size++;
    }
}
void * dequeue_list(struct list * q){
    void * ret_val=0;
    if(q->head!=0){
        struct node_list * to_remove=q->head;
        ret_val=q->head->content;
        if(q->head->next==0){
            q->head=0;
            q->tail=0;
        }else{
            q->head=q->head->next;
            q->head->prev=0;
        }
        free(to_remove);
        q->size--;
    }
    
    return ret_val;
}
void * peek_last_list(struct list * q){
    void * ret_val=0;
    if(q->tail!=0){
        ret_val=q->tail->content;
    }
    return ret_val;
}


void * pop_list(struct list * q){
    void * ret_val=0;
    if(q->tail!=0){
        struct node_list * to_remove=q->tail;
        ret_val=to_remove->content;
        q->tail=to_remove->prev;
        if(q->tail!=0){
            q->tail->next=0;
        }else{
            q->head=0;
        }
        free(to_remove);
        q->size--; 
    }
    return ret_val;
}
void print_list(struct list * q){
    struct node_list * n=q->head;
    while(n!=0){
        printf("%p ",n);
        n=n->next;
    }
    printf("\n");
}

int is_empty_list(struct list * q){
    return q->head==0;
}

void init_priority_queue(struct priority_queue * q){
    q->head=0;
    q->tail=0;
    q->size=0;
}

void insert_priority_queue(struct priority_queue * q,void * item, double val){
    struct  node_priority_queue * n=(struct node_priority_queue*)malloc(sizeof(struct node_priority_queue));
    n->content=item;
    n->value=val;
    if(q->head==0){ //if priority list is empty
        n->prev=0;
        n->next=0;
        q->head=n;
        q->size=1;
        q->tail=n;
    }else{
        struct node_priority_queue * n_current=q->head;
        while(n_current!=0&&n_current->value<val){//find the node we have to replace
            n_current=n_current->next;
        }
        if(n_current==0){//if it is the last, we put the node as last
            n->prev=q->tail;
            q->tail->next=n;
            n->next=0;
            q->tail=n;
        }else{//if it is not the last
            if(n_current->prev!=0){//if it is not the first
                n_current->prev->next=n;
                n->prev=n_current->prev;
                n_current->prev=n;
                n->next=n_current;
            }else{ //if it is the first
                q->head->prev=n;
                q->head=n;
                n->prev=0;
                n->next=n_current;
            }
            
        }
        q->size++;
    }
}


int main(){
    srand(0);
    struct priority_queue q;
    init_priority_queue(&q);
    int n=1;
    int i=0;
    
    for(i=0;i<10000;i++){
        int res=rand()%3;
        double x = (float)rand()/(float)(RAND_MAX/25);
        if(res==0){
            int * p=malloc(sizeof(int));
            int s1,s2;
            s1=q.size;
            insert_or_update_priority_queue(&q,p,x);
            s2=q.size;
            
            if(s2-s1>1 || s2-s1<0){
                // printf("Exiting\n");
                //exit(0);
            }
        }else if(res==1){
            insert_or_update_priority_queue(&q,&n,x);
        }else{
            dequeue_priority_queue(&q);
        }
    }
}

//TODO: at any time: check queue, order and size
void insert_or_update_priority_queue(struct priority_queue * q,void * item, double val){
    if(q->head==0){ //if priority list is empty
        struct  node_priority_queue * n=(struct node_priority_queue*)malloc(sizeof(struct node_priority_queue));
        n->content=item;
        n->value=val;
        n->prev=0;
        n->next=0;
        q->head=n;
        q->size=1;
        q->tail=n;
    }else{
        struct node_priority_queue * n_current=q->head;
        struct node_priority_queue * to_replace=0;
        struct node_priority_queue * actual_pos=0;
        while(n_current!=0&&(to_replace==0 || actual_pos==0)){//stop if we end queue
            if(n_current->content==item){
                actual_pos=n_current;
            }
            if(n_current->value>val&&to_replace==0){
                to_replace=n_current;
            }
            n_current=n_current->next;
        }
        struct  node_priority_queue * n=0;
        if(actual_pos!=0){//if node is already in queue
            if(actual_pos==to_replace){
                actual_pos->value=val;
                return;
            }
            n=actual_pos;
            actual_pos->value=val;
            if(actual_pos->prev!=0){
                actual_pos->prev->next=actual_pos->next;
            }else{
                q->head=actual_pos->next;
            }
            if(actual_pos->next!=0){
                actual_pos->next->prev=actual_pos->prev;
            }else{
                q->tail=actual_pos->prev;
            }
        }else{//if node is new, do the same as insert, except we already have address
            n=(struct node_priority_queue*)malloc(sizeof(struct node_priority_queue));
            n->content=item;
            n->value=val;
            q->size++;
            
        }
        if(to_replace==0){//if it is the last, we put the node as last
            n->prev=q->tail;
            q->tail->next=n;
            n->next=0;
            q->tail=n;
        }else{//if it is not the last
            if(to_replace->prev!=0){//if it is not the first
                to_replace->prev->next=n;
                n->prev=to_replace->prev;
                to_replace->prev=n;
                n->next=to_replace;
            }else{ //if it is the first
                
                q->head->prev=n;
                q->head=n;
                n->prev=0;
                n->next=to_replace;
            }
            
        }  
        
    }
}

void * dequeue_priority_queue(struct priority_queue * q){
    void * ret_val=0;
    if(q->head!=0){
        struct node_priority_queue * to_remove=q->head;
        ret_val=q->head->content;
        if(q->head->next==0){
            q->head=0;
            q->tail=0;
        }else{
            q->head=q->head->next;
            q->head->prev=0;
        }
        q->size--;
        free(to_remove);
    }
    return ret_val;
}
void print_priority_queue(struct priority_queue * q){
    struct node_priority_queue * n=q->head;
    while(n!=0){
        printf("%f ",n->value);
        n=n->next;
    } 
    printf("\n");
}

int is_empty_priority_queue(struct priority_queue * q){
    return q->head==0;
}
