#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definition of the type used to specify the pointer to a node of a list or queue */
typedef struct node* nodePtr;
/*dobbiamo inizializzarlo prima prechè
poi quando definiamo il nodo dobbiamo usare 
nodePtr*/

/* Definition of the Event Notice - typical fields to contain event’s attributes */
typedef struct {
    char type;
    char name[256];
    double create_time;
    double occur_time; 
    double arrival_time;  
    double service_time; 
} event_notice;

/* Definition of the Node for managing Event Notices in Lists and Queues */
struct node {
    event_notice event;    // Contains the event data
    nodePtr left;          // Pointer to the previous node in the doubly linked list
    nodePtr right;         // Pointer to the next node in the doubly linked list
};

nodePtr get_new_node(event_notice event) {
    // Allocate memory for the new node
    nodePtr new_node = (nodePtr)malloc(sizeof(struct node));

    // Check if memory allocation was successful
    if (new_node == NULL) {
        printf("Error: Memory allocation failed!\n");
        exit(EXIT_FAILURE); // Exit the program if malloc fails
    }

    // Initialize the new node with the given event
    new_node->event = event;
    new_node->left = NULL;  // Set the left pointer to NULL
    new_node->right = NULL; // Set the right pointer to NULL

    return new_node; // Return the pointer to the new node
};

void initialize() {
    /* Definition and Initialization of Useful Variables */
    double clock = 0;
    int halt = 0;

    /* Definition and Initialization of Future Event List (FEL) and additional structures */
    nodePtr FEL = NULL;  // Future Event List initialized to NULL
    nodePtr Queue1 = NULL;  // Queue of server 1 initialized to NULL (for managing events)
    nodePtr Queue1 = NULL;  // Queue of server 2 initialized to NULL (for managing events)
    /* First Arrival Time is 0 */
    double arrival_t = 0;  // Set the first arrival time to 0

    /* Extract the first Service Time as an Exponential Random Variable */
    double service_t = get_exponential_service_time();  // Assume this function generates the service time

    /* Initialize Event notice of first arrival and Schedule first event */
    nodePtr event_notice = get_new_node();  // Create a new node for the event
    event_notice->event.type = A;  // Set the type to ARRIVAL
    event_notice->event.occur_time = arrival_t;  // Set the event's occurrence time
    event_notice->event.arrival_time = arrival_t;  // Set the arrival time
    event_notice->event.service_time = service_t;  // Set the service time
    schedule(event_notice);  // Schedule this event in the Future Event List

    /* Initialize Event notice of End Simulation and Schedule last event */
    nodePtr end_event_notice = get_new_node();  // Create a new node for the end event
    end_event_notice->event.type = E;  // Set the type to END
    end_event_notice->event.occur_time = End_time;  // Set the end time (assuming you have this predefined)
    schedule(end_event_notice);  // Schedule the end simulation event in the Future Event List
}
