#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define N 10              // max number of machines
#define HETA_ARRIVAL 3000 // Expected value for arrivals
#define HETA_SHORT 40     // Expected value for short station
#define HETA_LONG 960     // Expected value for long station
#define MAX_EVENTS 100000000
#define BETA 0.2 // routing probability
#define MAX_TIME 10000000

/*------VARIABLES FOR THE SIMULATION CORE------*/
double clock = 0;
int halt=0;
int event_counter=0;
double heta;
double waiting_short = 0;
double waiting_long = 0;
double beta = BETA;
int nserv_s =0;
int nserv_l =0;
double heta_arrival = 3000;
double heta_short = 40;
double heta_long = 960;

/*------VARIABLES FOR THE REGENERATION METHOD------*/
int cycle_num = 0;
int cycle_in_group = 0;
double total_waiting_short = 0;
double total_waiting_long = 0;

typedef enum {
    AS = 1,
    AL = 2,
    DS = 3,
    DL = 4,
    END = 5
} event_label;
/* Definition of the type used to specify the pointer to a node of a list or queue */
typedef struct node* nodePtr;
/*dobbiamo inizializzarlo prima prechè
poi quando definiamo il nodo dobbiamo usare 
nodePtr*/

typedef struct DLL{
    nodePtr Head;
    nodePtr Tail;
} dll;


dll FEL = {NULL, NULL}; /* Pointer to the header of the Future Event List implemented with a doubly linked list */
dll IQ1 = {NULL, NULL}; /* Pointer to the header of the Future Event List implemented with a doubly linked list */
dll IQ2 = {NULL, NULL};

int node_number;
int job_number; /* (progressive) Job identification number */

/* Definition of the Event Notice - typical fields to contain event’s attributes */
typedef struct {
    event_label type;
    int machine_id;
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


nodePtr get_new_node() {
    // Allocate memory for the new node
    nodePtr new_node = (nodePtr)malloc(sizeof(struct node));

    // Check if memory allocation was successful
    if (new_node == NULL) {
        printf("Error: Memory allocation failed!\n");
        exit(EXIT_FAILURE); // Exit the program if malloc fails
    }

    // Initialize the new node with the given event
    new_node->event.type = 0;             // Default type
    new_node->event.machine_id = 0;    // Empty int
    new_node->event.create_time = 0.0;   // Default time
    new_node->event.occur_time = 0.0;    // Default time
    new_node->event.arrival_time = 0.0;  // Default time
    new_node->event.service_time = 0.0;  // Default time
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

/*
-----------------------------------------------------------------------------
-------------------------SIMULATION CORE-------------------------------------
-----------------------------------------------------------------------------
*/

void initialize() {
    /* Definition and Initialization of Useful Variables */
    clock = 0;
    halt = 0;

    /*we pick the 10 arrival times of the system. we create the nodes and schedule them!*/
    for (int i = 1; i < 11; i++) {
        nodePtr init_job = get_new_node();
        double arrival_t = exponential_random(heta_arrival);

        init_job->event.arrival_time = arrival_t;
        init_job->event.create_time = clock;
        init_job->event.type = AS;
        init_job->event.machine_id = i;
        schedule(init_job);
    }
   
    /* Initialize Event notice of End Simulation and Schedule last event */
    nodePtr end_event_notice = get_new_node(); 
    end_event_notice->event.type = END;  // Set the type to END
    end_event_notice->event.occur_time = MAX_TIME;  // Set the end time (assuming you have this predefined)
    schedule(end_event_notice);  // Schedule the end simulation event in the Future Event List
}

void engine(void){
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
        case AS : a_short(new_event);
        break;
        case AL : a_long(new_event);
        break;
        case DS : d_short(new_event);
        break;
        case DL : d_long(new_event);
        break;
        case END : end(new_event);
        break;
    }
    event_counter++;
}

void a_short(struct node* node_event){
    struct node* next_job;
    nserv_s++;
    node_event->event.create_time = clock;

    /*pick a service time*/
    node_event->event.service_time = exponential_random(heta_short);

    /*idle... or busy?*/
    if (nserv_s == 1) { //idle!
        node_event->event.type = DS;
        node_event->event.occur_time = clock + node_event->event.service_time;
        schedule(node_event);
    } else { //busy!
        node_event->event.type = DS;
        node_event->event.occur_time = 0; //initialize it
        enqueue(&IQ1, node_event);
    }
}

void a_long(struct node* node_event) {
    struct node* next_job;
    nserv_l++;
    node_event->event.create_time = clock;

    /*pick a service time*/
    node_event->event.service_time = exponential_random(heta_long);

    /*idle... or busy?*/
    if (nserv_s == 1) { //idle!
        node_event->event.type = DL;
        node_event->event.occur_time = clock + node_event->event.service_time;
        schedule(node_event);
    } else { //busy!
        node_event->event.type = DL;
        node_event->event.occur_time = 0; //initialize it
        enqueue(&IQ2, node_event);
    }
}

void d_short(struct node* node_event){
    double waiting_time;
    nodePtr next_job;

    /*compute our statistics and waiting times!*/
    nserv_s--;
    waiting_time = clock - node_event->event.arrival_time;
    waiting_short = waiting_short + waiting_time; 

    /*decide whether the customer will go to long or short repair station*/
    double decide_route = (rand() / (double)RAND_MAX);
    if (decide_route <= beta) {
        /* go to the long station.
        this means that our d_short becomes a a_long*/
        node_event->event.type = AL;
        node_event->event.arrival_time = node_event->event.occur_time;
        if (nserv_s>0) {
            /*extract from queue of server 1 and schedule departure*/
            next_job = dequeue(&IQ1);
            next_job->event.type = DS;
            /*pick service time*/
            next_job->event.service_time = exponential_random(heta_short);
            next_job->event.occur_time = clock + next_job->event.service_time;
            schedule(next_job);
        }

    } else {
        /*go back to the pool. schedule its next arrival at short station*/
        node_event->event.type = AS;
        node_event->event.arrival_time = clock + exponential_random(heta_arrival);
        node_event->event.occur_time = node_event->event.arrival_time;
        schedule(node_event);
    }
}

void d_long(struct node* node_event){
    double waiting_time;
    struct node* next_job;

    /*compute our statistics and waiting times!*/
    nserv_l--;
    waiting_time = clock - node_event->event.arrival_time;
    waiting_long = waiting_long + waiting_time; 

    /*go back to the pool. schedule its next arrival at short station*/
    node_event->event.type = AS;
    node_event->event.arrival_time = clock + exponential_random(heta_arrival);
    node_event->event.occur_time = node_event->event.arrival_time;
    schedule(node_event);
}

void end(struct node* node_event){
    halt = 1;
}

void return_node(nodePtr node_event) {
    free(node_event); // Deallocate the memory used by the node
}

/*
|-------------------------------------------------------------------------------|
|--------------------------------DATA STRUCTURES--------------------------------|
|-------------------------------------------------------------------------------|
*/


void schedule(struct node* node_event){
/* empty fel, head is null */
if (FEL.Head == NULL) {
    FEL.Head = node_event;
    FEL.Tail = node_event;
    node_event->left = NULL;
    node_event->right = NULL;
    return;
}

/*insert at the front*/
if (node_event->event.occur_time <= FEL.Head->event.occur_time) {
    node_event->right = FEL.Head;
    node_event->left = NULL;
    FEL.Head->left = node_event;
    FEL.Head = node_event;
    return;
}

/*insert in the middle or end: needed traversal*/

struct node* current = FEL.Head;

/*traverse*/
while (current != NULL && node_event->event.occur_time > current->event.occur_time) {
    current = current->right;
}

/*insert*/
if (current != NULL) {
    node_event->right = current;
    node_event->left = current->left;
    if (current->left != NULL) {
        current->left->right = node_event;
    }
    current->left = node_event;
} else {
    /*insert at the end*/
    node_event->left = FEL.Tail;
    node_event->right = NULL;
    FEL.Tail->right = node_event;
    FEL.Tail = node_event;
}
}

struct node* event_pop(void){
/*check if the FEL is empty*/
if (FEL.Head == NULL) {
    printf("the future event list is empty. I popped all there was to pop!");
    return NULL;
}

/*save temporarily first node*/
struct node* first_node = FEL.Head;

/*update the head of the FEL to point to the next node*/
FEL.Head = FEL.Head->right;

/*if the list becomes empty update the tail of the FEL*/
if (FEL.Head == NULL) {
    FEL.Tail = NULL; // no nodes left
} else {
    FEL.Head->left = NULL; //detach popped node
}

/*detach pointers*/
first_node->left = NULL;
first_node->right = NULL;

return first_node;
}

void enqueue(dll* IQ, struct node* new_inqueue){
    /*check if the input node is null*/
    if (new_inqueue == NULL) {
        printf("why did you give me a null node?");
        return;
    }

    /*init pointers of the new_inqueue*/
    new_inqueue->left = NULL;
    new_inqueue->right = NULL;

    if (IQ->Head == NULL && IQ->Tail == NULL) {
        /*if the queue is empty insert first node*/
        IQ->Head = new_inqueue;
        IQ->Tail = new_inqueue;
    } else {
        /*link new node at the end of the Q*/
        new_inqueue->left = IQ->Tail; // its previous node is the tail
        IQ->Tail->right = new_inqueue; // it is the successor of the tail
        IQ->Tail = new_inqueue; // it is the new tail!
    }
}

struct node* dequeue(dll* IQ) {
    /*check if valid q*/
    if (IQ == NULL && IQ->Head == NULL) {
        printf("I can't dequeue from an empty/nonexistent queue!");
        return NULL;
    }

    /*get first node*/
    struct node* dequeued_node = IQ->Head;

    /*update head to point to the second node*/
    IQ->Head = dequeued_node->right;

    if (IQ->Head != NULL) {
        /*update the head if there are events left in the q*/
        IQ->Head->left = NULL;
    } else {
        /*if the queue is empty also the tail is null*/
        IQ->Tail = NULL;
    }

    /*disconnect the dequeued node from the queue*/
    dequeued_node->left = NULL;
    dequeued_node->right = NULL;

    printf("dequeued node.\n");
}

/*
|--------------------------------------------------------------------------|
|--------------------------REGENERATION UTILITIES--------------------------|
|--------------------------------------------------------------------------|
*/

int RegPoint(nodePtr node_event, int *cycle_in_group, int *cycle_num) {
    /*
    in this scenario every departure from any station may be a suitable regeneration point. 
    since we are interested in the departure from the long station we pick as regeneration points the departures from the long station.
    in order to preserve the conditions to apply the central limit theorem we have to have a reasonable sample size and so we must group different regeneration cycles together.
    we choose 50 as our sample size, since the minimal number of samples commonly used as guideline is 30.

    this function returns 1 (true) for regeneration point, 0 (false) otherwise
    */
   if (*cycle_in_group <= 50)
   {
    if (node_event->event.type == DL) {
    (*cycle_in_group)++;
    return 0;
    }
   } else {
    (*cycle_num)++;
    *cycle_in_group = 0;
    return 1;
   }
}

void CollectRegStatistics(double *waiting_long, double *total_waiting_long){
    *total_waiting_long += *waiting_long;
}

void ResetMeasures(double *waiting_long){
    *waiting_long == 0;
}

/*todo: compute confidence intervals and decide 2 stop*/