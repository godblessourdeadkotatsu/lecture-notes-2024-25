#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double clock = 0;
int halt=0;
int event_counter=0;
int narr_short;
int narr_long;
double heta;
double Sum_w = 0;
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

// NON DOBBIAMO SCHEDULARE LE PARTENZE IN CODA
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

double exponential_random(double heta) {
  double unif = (double)rand() / RAND_MAX;
  if ((1 - unif) < 1e-20) {
    unif = 1e-20; // this will give us a limit in the case in which the argument
                  // of the logarithm is too close to 0 (and thus exploding)
  }
  double exp = (-heta * logf(1 - unif));
  // double pick = 1- ((double)rand() / RAND_MAX);
  return exp;
}


void initialize() {
    /* Definition and Initialization of Useful Variables */
    clock = 0;
    halt = 0;

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
    event_notice->event.type = 'A';  // Set the type to ARRIVAL
    event_notice->event.occur_time = arrival_t;  // Set the event's occurrence time
    event_notice->event.arrival_time = arrival_t;  // Set the arrival time
    event_notice->event.service_time = service_t;  // Set the service time
    schedule(event_notice);  // Schedule this event in the Future Event List

    /* Initialize Event notice of End Simulation and Schedule last event */
    nodePtr end_event_notice = get_new_node();  // Create a new node for the end event
    end_event_notice->event.type = 'E';  // Set the type to END
    end_event_notice->event.occur_time = End_time;  // Set the end time (assuming you have this predefined)
    schedule(end_event_notice);  // Schedule the end simulation event in the Future Event List
}

void engine(void ){
char event_type;
double oldclock;
double interval;
nodePtr new_event; // pointer to struct node

/* get the 1st event from future event list */
new_event = event_pop(); /* TODO: EVENT POP */

/* update clock */
oldclock = clock;
clock = new_event->event.occur_time;
interval = clock - oldclock;

/* collect statistics */

/* identify and process current event */
event_type = new_event->event.type;
switch(event_type) {
    case 'ARRIVAL' : arrival(new_event);
    break;
    case 'DEPARTURE' : departure(new_event);
    break;
    case 'END' : end(new_event);
    break;
}
event_counter++;
}

void arrival(struct node* node_event){
    double delta_t;
    double cycle_time;
    struct node* next_job;

    /*update statistics*/
    narr_short++;
    /*idk*/
    node_event->event.create_time = clock;
    if (narr_short == 1) {
        /* process arrival at busy server */
        node_event->event.type = 'DEPARTURE'; /*transform into departure */
        node_event->event.occur_time = clock + node_event->event.service_time; 
        schedule(node_event); /* todo schedule */
    }
    else {
        /* process arrival at empty server */
        enqueue(node_event); /* todo enqueue */
    }

    double arrival_t = exponential_random(heta);
    double service_t = exponential_random(heta);
    /* schedule next arrival */
    next_job = get_new_node();
    next_job->event.type = 'ARRIVAL';
    next_job->event.create_time = clock;
    next_job->event.occur_time = arrival_t;
    next_job->event.arrival_time = arrival_t;
    next_job->event.service_time = service_t;
    next_job->left = NULL;
    next_job-> right = NULL;
    schedule(next_job);
}
void departure(struct node* node_event){
    double waiting_time;
    struct node* next_job;

    /*upd statistics*/
    narr_short--;
    waiting_time = clock - node_event->event.arrival_time;
    Sum_w = Sum_w + waiting_time; 

    if (narr_short>0) {
        /*process dep from a server with a queue*/
        next_job = dequeue(); 
        next_job->event.type = 'DEPARTURE';
        next_job->event.occur_time = clock + next_job->event.service_time;
        schedule(next_job);
    }
    return_node(node_event);
}

void end(struct node* node_event){
    halt = 1;
    return_node(node_event);
}