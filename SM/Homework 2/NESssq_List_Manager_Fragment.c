
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include "NESssq.h"
#include "NESssq_List_Manager.h"




void enqueue(nodePtr new_node, dll * curr_queue){
/* function to add an element at the tail of a generic queue (curr_queue) */
	if(curr_queue->Tail == NULL)
	    /* curr_queue is empty*/  
	    curr_queue->Head = new_node;
	else
	    /* add at the end of a non-empty curr_queue */
	    curr_queue->Tail->right = new_node;
	/* adjust pointers */
	new_node->left = curr_queue->Tail;
	new_node->right = NULL;
	curr_queue->Tail = new_node;	
}

nodePtr dequeue(dll * curr_queue){
/* function to remove the element at the head of a generic queue (curr_queue) */
	nodePtr item;   
			if(curr_queue->Head == NULL) 
			/* curr_queue is empty */
			    return NULL;
			/* point to the element being removed from curr_queue */
			item =curr_queue->Head;
			if(curr_queue->Head->right == NULL){
				/* curr_queue contains only one element that is being removed 
				(leaving curr_queue empty) */
				curr_queue->Head = NULL;
				curr_queue->Tail = NULL;
			}
			else{
				/* adjust pointers to the new head of curr_queue */
				curr_queue->Head = curr_queue->Head->right;
				curr_queue->Head->left = NULL;
			}
	/* clear the returned node*/
	item->left = NULL;
	item->right = NULL;
	return item;
}














