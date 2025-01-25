/* -------------------------------------------------------------------------
Università di Torino
M.S. in STOCHASTICS AND DATA SCIENCE
Course in Simulation
Homework 7
Thrid exercise

By Andrea Crusi and Lorenzo Sala
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define N 10              // max number of machines
#define HETA_ARRIVAL 3000 // Expected value for arrivals
#define HETA_SHORT 40     // Expected value for short station 
#define BETA 0.2 // routing probability
#define MAX_TIME 10000000

int verbose = 0; 

/*------VARIABLES FOR THE SIMULATION CORE------*/
double sim_clock = 0;
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
double alpha[] = {0.95,0.05};
double mu[] = {10,19010};

/*------VARIABLES FOR THE REGENERATION METHOD------*/
int cycle_num = 1;
int cycle_in_group = 0;
double total_waiting_short = 0;
double total_waiting_long = 0;
double total_waiting_short_squared = 0;
double S_AA = 0;
double S_Anu = 0;
double S_nu = 0;
double S_nunu = 0;
double error = 0;
double r_hat = 0;

typedef enum {
    AS = 1,
    AL = 2,
    DS = 3,
    DL = 4,
    END = 5
} event_label;
/* Definition of the type used to specify the pointer to a node of a list or queue */
typedef struct node* nodePtr;

typedef struct DLL{
    nodePtr Head;
    nodePtr Tail;
} dll;

/*we define the lists that we will use*/

dll FEL = {NULL, NULL}; 
dll IQ1 = {NULL, NULL};
dll IQ2 = {NULL, NULL};


/* we define the event notice structure */
typedef struct {
    event_label type;
    int machine_id;
    double create_time;
    double occur_time; 
    double arrival_time;  
    double service_time; 
} event_notice;

/* definition of the Node for managing Event Notices in lists and queues */
struct node {
    event_notice event;   
    nodePtr left;          
    nodePtr right;         
};

/* function declaration */
nodePtr get_new_node();
double exponential_random(double heta);
double hyperexponential_random(int k, double* alpha, double* heta);
void initialize();
void engine(void);
void a_short(struct node* node_event);
void a_long(struct node* node_event);
void d_short(struct node* node_event);
void d_long(struct node* node_event);
void end(struct node* node_event);
void release_nodes(nodePtr *head);
void report(double sim_duration);
void schedule(struct node* node_event);
struct node* event_pop(void);
void enqueue(dll* curr_queue, struct node* new_node);
struct node* dequeue(dll* curr_queue);
void RegPoint(
    nodePtr node_event, 
    int *cycle_in_group, 
    int *cycle_num,
    double *waiting_long, 
    double *total_waiting_long, 
    double *S_AA,
    double *S_Anu,
    double *S_nu,
    double *S_nunu,
    double *r_hat
    );
void CollectRegStatistics(
    double *waiting_long, 
    double *total_waiting_long, 
    double *S_AA,
    double *S_Anu,
    double *S_nu,
    double *S_nunu,
    int *cycle_in_group
    );
void ResetMeasures(double *waiting_long, int *cycle_in_group);
double ComputeConfidenceIntervals(
    double *r_hat, 
    double *total_waiting_long,
    int *cycle_num, 
    double *S_AA, 
    double *S_Anu, 
    double *S_nunu, 
    double *S_nu
    );
int DecideToStop(int cycle_num, double error, double r_hat);
int isNumber(const char *str);



nodePtr get_new_node() {
    // allocate memory for the new node
    nodePtr new_node = (nodePtr)malloc(sizeof(struct node));

    // check if memory allocation was successful
    if (new_node == NULL) {
        printf("Error: Memory allocation failed!\n");
        exit(EXIT_FAILURE); // Exit the program if malloc fails
    }

    // initialize the new node 
    new_node->event.type = 0;      
    new_node->event.machine_id = 0; 
    new_node->event.create_time = 0.0;  
    new_node->event.occur_time = 0.0;  
    new_node->event.arrival_time = 0.0; 
    new_node->event.service_time = 0.0; 
    new_node->left = NULL;  
    new_node->right = NULL; 

    return new_node; 
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

double hyperexponential_random(int k, double* alpha, double* heta) {
  // Cumulative distribution for alpha
  double cumulative_alpha[k];
  cumulative_alpha[0] = alpha[0]; 
  // Simply add all the alphas parameter (they sum to 1)
  for (int i = 1; i < k; i++) {
    cumulative_alpha[i] = cumulative_alpha[i - 1] + alpha[i];
  }

  // Generate a uniform random number
  double Y = (double)rand() / RAND_MAX;

  // Select the component distribution: find which  distribution corresponds has a cumulative probability that corresponds to the drawn uniform variable
  int j = 0;
  while (Y > cumulative_alpha[j]) {
    j++;
  }

  // Generate a random variable from the chosen exponential distribution
  double X = exponential_random(heta[j]);

  return X;
}
/*
-----------------------------------------------------------------------------
-------------------------SIMULATION CORE-------------------------------------
-----------------------------------------------------------------------------
*/

/*function to print fel for debugging*/
void print_fel() {
    struct node* temp = FEL.Head;
    printf("FEL: ");
    while (temp != NULL) {
        printf("[Event Time: %f, Machine ID: %d, Event Type: %d] -> ", temp->event.occur_time, temp->event.machine_id, temp->event.type);
        temp = temp->right;
    }
    printf("NULL\n\n");
}

void initialize() {
    /* Definition and Initialization of Useful Variables */
    sim_clock = 0;
    halt = 0;

    FEL.Head = NULL;
    FEL.Tail = NULL;
    IQ1.Head = NULL;
    IQ2.Head = NULL;
    IQ1.Tail = NULL;
    IQ2.Tail = NULL;

    /*we pick the 10 arrival times of the system. we create the nodes and schedule them!*/
    for (int i = 1; i <= 10; i++) {
        nodePtr init_job = get_new_node();
        double arrival_t = exponential_random(heta_arrival);

        init_job->event.arrival_time = arrival_t;
        init_job->event.occur_time = arrival_t;
        init_job->event.create_time = sim_clock;
        init_job->event.type = AS;
        init_job->event.machine_id = i;
        schedule(init_job); 
    }

    verbose ? print_fel() : 0;
   
    /* now we prepare the event notice for the end event */
    nodePtr end_event_notice = get_new_node(); 
    end_event_notice->event.type = END; 
    end_event_notice->event.occur_time = MAX_TIME; 
    schedule(end_event_notice);
}

void engine(void){
    char event_type;
    double old_sim_clock;
    double interval;
    nodePtr new_event; // pointer to struct node

    /* get the 1st event from future event list */
    new_event = event_pop(); 

    /* update clock */
    old_sim_clock = sim_clock;
    sim_clock = new_event->event.occur_time;
    interval = sim_clock - old_sim_clock;

    verbose ? printf("Now processing an event of type %d with occur time %f for machine %d\n", new_event->event.type, new_event->event.occur_time, new_event->event.machine_id) : 0;

    RegPoint(new_event, &cycle_in_group, &cycle_num, &waiting_long, &total_waiting_long, &S_AA, &S_Anu, &S_nu, &S_nunu, &r_hat);

    /* identify and process current event */
    event_type = new_event->event.type;
    switch(event_type) {
        case AS : 
        verbose ? printf("Now processing an arrival to the short station (event %d)\n", new_event->event.type) : 0;
        a_short(new_event);
        break;
        case AL : 
        verbose ? printf("Now processing an arrival to the long station (event %d)\n", new_event->event.type) : 0;
        a_long(new_event);
        break;
        case DS : 
        verbose ? printf("Now processing a departure from the short station (event %d)\n", new_event->event.type) : 0;
        d_short(new_event);
        break;
        case DL : 
        verbose ? printf("Now processing a departure from the station (event %d)\n", new_event->event.type) : 0;
        d_long(new_event);
        break;
        case END : 
        verbose ? printf("Now processing the end (event %d)\n", new_event->event.type) : 0;
        end(new_event);
        break;
    }
    verbose ? print_fel() : 0;
    verbose ? printf("Number of clients in Queue 1: %d\nNumber of clients in Queue 2: %d\n\n", nserv_s, nserv_l) : 0;
    event_counter++;
}

void a_short(struct node* node_event){
    struct node* next_job;
    nserv_s++;
    node_event->event.create_time = sim_clock;

    /*pick a service time*/
    node_event->event.service_time = exponential_random(heta_short);
    verbose ? printf("Extracted a short departure time of %f\n", node_event->event.service_time) : 0;

    /*idle... or busy?*/
    if (nserv_s == 1) { //idle!
        node_event->event.type = DS;
        node_event->event.occur_time = sim_clock + node_event->event.service_time;
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
    node_event->event.create_time = sim_clock;

    /*pick a service time*/
    node_event->event.service_time = hyperexponential_random(2,alpha,mu);
    verbose ? printf("Extracted a long departure time of %f\n", node_event->event.service_time) : 0;

    /*idle... or busy?*/
    if (nserv_l == 1) { //idle!
        node_event->event.type = DL;
        node_event->event.occur_time = sim_clock + node_event->event.service_time;
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
    waiting_time = sim_clock - node_event->event.arrival_time;
    waiting_short = waiting_short + waiting_time; 

    /*decide whether the customer will go to long or short repair station*/
    double decide_route = (rand() / (double)RAND_MAX);
    if (decide_route <= beta) {
        /* go to the long station.
        this means that our d_short becomes a a_long*/
        node_event->event.type = AL;
        node_event->event.arrival_time = node_event->event.occur_time;
        verbose ? printf("Bad luck! Machine %d gets a long repair!\n", node_event->event.machine_id) : 0;
        schedule(node_event);
    } else {
        verbose ? printf("Good luck! Machine %d goes back in the pool!\n", node_event->event.machine_id) : 0;
        /*go back to the pool. schedule its next arrival at short station*/
        node_event->event.type = AS;
        node_event->event.arrival_time = sim_clock + exponential_random(heta_arrival);
        node_event->event.occur_time = node_event->event.arrival_time;
        schedule(node_event);
    } 
    /*now check the queue. if the queue is not empty then dequeue the first node and transform it into a departure from this short station*/
            if (nserv_s>0) {
            verbose ? printf(" dequeuing next job from the queue 1\n") : 0;
            /*extract from queue of server 1 and schedule departure*/
            next_job = dequeue(&IQ1);
            next_job->event.type = DS;
            /*pick service time*/
            next_job->event.service_time = exponential_random(heta_short);
            next_job->event.occur_time = sim_clock + next_job->event.service_time;
            schedule(next_job);
        }
}

void d_long(struct node* node_event){
    double waiting_time;
    struct node* next_job;

    /*compute our statistics and waiting times!*/
    nserv_l--;
    waiting_time = sim_clock - node_event->event.arrival_time;
    waiting_long = waiting_long + waiting_time; 

    /*modify this node to be the next AS*/
    node_event->event.type = AS;
    double break_time = exponential_random(heta_arrival);
    node_event->event.occur_time = sim_clock + break_time;
    node_event->event.arrival_time = node_event->event.occur_time;
    schedule(node_event);

    /*again: check the queue. if it is not empty then dequeue the first node
    and stage it to be the next departure from the long station*/
    if (nserv_l>0) {
        next_job = dequeue(&IQ2);
        verbose ? printf("queue long station not empty. dequeuing job of machine %n\n", next_job->event.machine_id) : 0;
        next_job->event.type = DL;
        /*pick service time*/
        double service_long = hyperexponential_random(2,alpha,mu);
        next_job->event.service_time = service_long;
        next_job->event.occur_time = sim_clock + service_long;
        schedule(next_job);
    }
}

void end(struct node* node_event){
    halt = 1;
}

void release_nodes(nodePtr *head) {
    /*traverse the lists and free the nodes*/
    nodePtr temp;
    while (*head != NULL) {
        temp = *head;
        *head = (*head)->right;
        free(temp);
    }
}

void report(double sim_duration) {
    printf("%f,%f\n", r_hat-error, r_hat+error);
    release_nodes(&FEL.Head);
    release_nodes(&IQ1.Head);
    release_nodes(&IQ2.Head);
}

/*
|-------------------------------------------------------------------------------|
|--------------------------------DATA STRUCTURES--------------------------------|
|-------------------------------------------------------------------------------|
*/


void schedule(struct node* node_event){
/* empty fel, head is null */
if (FEL.Head == NULL) {
    FEL.Head = node_event; //the tail and the head are now the first node
    FEL.Tail = node_event;
    node_event->left = NULL;
    node_event->right = NULL;
    verbose ? printf("Scheduled event of type %d at occur time %f for machine %d. EMPTY Q, INSERT HEAD\n", node_event->event.type, node_event->event.occur_time, node_event->event.machine_id) : 0;
    return;
}

/*insert at the front*/
if (node_event->event.occur_time <= FEL.Head->event.occur_time) {
    node_event->right = FEL.Head;
    node_event->left = NULL; 
    FEL.Head->left = node_event;
    FEL.Head = node_event; //now the head is the current event we are scheduling
    verbose ? printf("Scheduled event of type %d at occur time %f for machine %d. INSERTION AT THE FRONT\n", node_event->event.type, node_event->event.occur_time, node_event->event.machine_id) : 0;
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
verbose ? printf("Scheduled event of type %d at occur time %f for machine %d\n", node_event->event.type, node_event->event.occur_time, node_event->event.machine_id) : 0;
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

verbose ? printf("Popped node from the FEL. This event was of the type %d and had an occur time of %f\n", first_node->event.type, first_node->event.occur_time) : 0;
return first_node;
}

void enqueue(dll* curr_queue, struct node* new_node){
    /* function to add an element at the tail of a generic queue (curr_queue) */
	if(curr_queue->Tail == NULL) {
	    /* curr_queue is empty*/  
	    curr_queue->Head = new_node;
        verbose ? printf("the current queue was empty. added node of machine %d as new head\n", new_node->event.machine_id) : 0;
	} else {
	    /* add at the end of a non-empty curr_queue */
	    curr_queue->Tail->right = new_node;
        verbose ? printf("the current queue was NOT empty. added node of machine %d as new tail\n", new_node->event.machine_id) : 0;
    }
	/* adjust pointers */
	new_node->left = curr_queue->Tail;
	new_node->right = NULL;
	curr_queue->Tail = new_node;	
}

nodePtr dequeue(dll * curr_queue){
/* function to remove the element at the head of a generic queue (curr_queue) */
	nodePtr item;   
			if(curr_queue->Head == NULL) {
			/* curr_queue is empty */
                verbose ? printf("Empty Q. Nothing do dequeue.\n") : 0;
			    return NULL;
            }
			/* point to the element being removed from curr_queue */
			item = curr_queue->Head;
			if(curr_queue->Head->right == NULL){
				/* curr_queue contains only one element that is being removed 
				(leaving curr_queue empty) */
				curr_queue->Head = NULL;
				curr_queue->Tail = NULL;
                verbose ? printf("There was only element in the queue, about machine %d. I dequeued it.\n", item->event.machine_id) : 0;
			}
			else{
				/* adjust pointers to the new head of curr_queue */
				curr_queue->Head = curr_queue->Head->right;
				curr_queue->Head->left = NULL;
                verbose ? printf("About machine %d. I dequeued it.\n", item->event.machine_id) : 0;
			}
	/* clear the returned node*/
	item->left = NULL;
	item->right = NULL;
	return item;
}

/*
|--------------------------------------------------------------------------|
|--------------------------REGENERATION UTILITIES--------------------------|
|--------------------------------------------------------------------------|
*/

void RegPoint(
    nodePtr node_event, 
    int *cycle_in_group, 
    int *cycle_num,
    double *waiting_long, 
    double *total_waiting_long, 
    double *S_AA,
    double *S_Anu,
    double *S_nu,
    double *S_nunu,
    double *r_hat
    ) {
    /*
    in this scenario every departure from any station may be a suitable regeneration point. 
    since we are interested in the departure from the long station we pick as regeneration points the departures from the long station.
    in order to preserve the conditions to apply the central limit theorem we have to have a reasonable sample size and so we must group different regeneration cycles together.
    we choose 60 as our sample size, since the minimal number of samples commonly used as guideline is 30.
    */
   if (node_event->event.type == DL) {
    if (*cycle_in_group < 100) {
        (*cycle_in_group)++;
    } else {
        (*cycle_num)++;
        CollectRegStatistics(
        waiting_long, 
        total_waiting_long, 
        S_AA,
        S_Anu,
        S_nu,
        S_nunu,
        cycle_in_group
        ); 
        ResetMeasures(waiting_long, cycle_in_group);
        ComputeConfidenceIntervals(
        r_hat, 
        total_waiting_long,
        cycle_num, 
        S_AA, 
        S_Anu, 
        S_nunu, 
        S_nu
        ); 
    }
   }
}

void CollectRegStatistics(
    double *waiting_long, 
    double *total_waiting_long, 
    double *S_AA,
    double *S_Anu,
    double *S_nu,
    double *S_nunu,
    int *cycle_in_group
    ) {
        /*simply use the cumulative forumlae*/
    *total_waiting_long += *waiting_long;
    *S_AA += pow(*waiting_long, 2);
    *S_Anu += *waiting_long * *cycle_in_group;
    *S_nu += *cycle_in_group;
    *S_nunu += pow(*cycle_in_group, 2);
    verbose ? printf("%f\n", *S_nu) : 0;
}

void ResetMeasures(double *waiting_long, int *cycle_in_group){
    *waiting_long = 0;
    *cycle_in_group = 0;
}

double ComputeConfidenceIntervals(
    double *r_hat, 
    double *total_waiting_long,
    int *cycle_num, 
    double *S_AA, 
    double *S_Anu, 
    double *S_nunu, 
    double *S_nu
    ) {
        /*as before, simply use the cumulative formulae*/
    *r_hat = *total_waiting_long / *S_nu;
    double delta = sqrt(*cycle_num/(*cycle_num-1))*(sqrt(*S_AA-2* *r_hat* *S_Anu+pow(*r_hat, 2)* *S_nunu))/(*S_nu);
    error = 1.96 * delta;
    return(error);
}

int DecideToStop(int cycle_num, double error, double r_hat){
    /*
    this function returns 1 when the two conditions for stopping the regeneration are satisfied:
    1) at least 40 cycles;
    2) the error must not be greater than 10% of the point estimate
    */
    double error_percentage = 2*error/r_hat;
    return(cycle_num > 40 && error_percentage < 0.10);  
}

int isNumber(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

int main(int argc, char *argv[]){
   if (argc != 2 || !isNumber(argv[1])) {
        printf("Usage: %s <positive integer>\n", argv[0]);
        return 1;
    }

    /*
    space the argument so that the sequence of random seeds provides more variability.
    */
    int seed = (atoi(argv[1])) * 10;

    srand(seed); 

    initialize(); // do the initialization
    clock_t start_time = clock(); // start the stopwatch
    
    /*simulate*/
    while ( //we handle the two end conditions: the regeneration method must be enough to stop and the end event must be reached.
        halt == 0 || DecideToStop(cycle_num, error, r_hat) == 0
        /*we may just use decide to stop to stop the simulation, but by doing so we can choose to increase MAX_TIME to keep simulating and lowering the error.*/
    ) {
        engine();
    }
    
    clock_t end_time = clock();
    // print some statistics
    double sim_duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    report(sim_duration);

    return 0;
}