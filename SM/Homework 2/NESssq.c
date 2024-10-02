
/* -------------------------------------------------------------------------
 * This program simulates a single-server FIFO service node using arrival
 * times and service times read from a text file.  The server is assumed
 * to be idle when the first job arrives.  All jobs are processed completely
 * so that the server is again idle at the end of the simulation.   The
 * output statistics are the average interarrival time, average service
 * time, average delay in the queue, average wait in the service 
 * node.
 * The program uses the Next_Event_Simulation approach with the implementation 
 * of a Future Event List (FEL). 
 * Scheduling the arrivals of new jobs and their 
 * completions when they are served after (possibly) spending time in the input 
 * queue sets the ground for simulating single server systems of this type with
 * different queueing and service policies.
 * The simulation stops when an END_Simulation event "emerges" from the FEL, 
 * meaning that all the arrived jobs have been processed. To ensure this to occurr 
 * correctly an END-of-Simulation event is scheduled during the initialization
 * of the simulator for an extremely large End-of-Simulation time.
 
 *
 * Name              : NESssq.c  (Next_Event Simulation of Single Server Queue)
 * Authors           : Gianfranco Balbo
 * Language          : ANSI C
 * Latest Revision   : 8-12-2017
 * ------------------------------------------------------------------------- 
 */
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include "NESssq.h"
#include "NESssq_List_Manager.h"


double Start;         /* Beginnig of Observation Period */
double Stop;          /* End of Observation Period */
double ObservPeriod;  /* Length of the Observation Period */
double clock;         /* Clock of the simulator - Simulation time */ 
double lasttime;      /* time of last processed event*/
double End_time;      /* maximum simulation time */

double arrival_t;     /* time of arrival */
double service_t;     /* service time read from trace file*/

boolean halt;         /* End of simulation flag */
int nsys;             /* Number of customers in system */
int nsysMax;          /* Maximum numbwr of customers in system */
int narr;             /* Number of Arrivals */
int ncom;             /* Number of Completions */
int ndelayed;         /* Number of Delayed customers */

int event_counter;    /* Number of events processed by the simulkator*/
int node_number;      /* actual number of nodes used by the simulator */
int return_number;    /* Number of nodes used by the simulator */



int main(int argc, char* argv[]){
	int i;
	fp = fopen(FILENAME, "r");
    if (fp == NULL) {
       fprintf(stderr, "Cannot open input file %s\n", FILENAME);
       getchar();
       return (1);
    }
    /* Simulation Run */
    if (!feof(fp)) {
	   simulate();
	   getchar();
       return 0;
    }   
}

void simulate() {
	/* Simulation core */
	initialize();
	while (!(halt))
	   engine(); 
	report();
}


void engine(void){
	int event_type;
	double  oldclock;
	double  interval;
	nodePtr new_event;
	
	/* Get the first event notice from Future Event List */
	new_event = event_pop();


	/* update clock */

	clock = new_event->event.occur_time;

	
	
	/* Identify and process current event */
	event_type = new_event->event.type;	
	switch(event_type){
		case ARRIVAL : arrival(new_event);
		break;
		case DEPARTURE : departure(new_event);
		break;
		case END : 	end(new_event);
		break;
	}
	event_counter++;
	lasttime =clock;
}



void initialize(){
	/* event notice used to store in the Future Event List the 
	   records corresponding to the events known to occur (in the future)
	   at the moment of the initialization */
	nodePtr curr_notice;
	double inter_arrival_time;

    /* Control Settings  */
	halt          = false;
	/* Basic  counters  */
	job_number    = 1;
	event_counter = 0;
	node_number   = 0;

	/* Future Event List */
	FEL.Head = NULL;
	FEL.Tail = NULL;
	/* Input Queue of Server */
	Input_Queue.Head = NULL;
	Input_Queue.Tail = NULL;
	/* Available List */
	AL.Head = NULL;
	AL.Tail = NULL;	
	

    /* Basic Statistic Measures  */
	nsys             = 0;
	nsysMax          = 0;
	narr             = 0;
	ncom             = 0;
	ndelayed         = 0;
	clock            = 0.0;   
	
	/* Set Maximum Simulation length */
	End_time          = ENDTIME;
	
	printf("\n-------------------------------------------------------------------------");	
	 
	if (!feof(fp)) {
		
			
		/* Get first Arrival and Service Time pair*/
		arrival_t = clock + GetInterArrival(fp);
		service_t = GetService(fp);
		
	   	/* Initialize Event notice of first arrival and Schedule first event */
	   	curr_notice = get_new_node();
	   	curr_notice->event.type = ARRIVAL;
	   	curr_notice->event.create_time =clock;
	   	curr_notice->event.occur_time = arrival_t;
	   	curr_notice->event.arrival_time = arrival_t;
	   	curr_notice->event.service_time = service_t;
   	   	curr_notice->right = NULL;
	   	curr_notice->left = NULL;
	   	sprintf(curr_notice->event.name, "J%d", (job_number++));
	   	schedule(curr_notice);
	   
	   	/* Initialize Event notice of End Simulation and Schedule last event*/
	   	curr_notice = get_new_node();
	   	curr_notice->event.type = END;
	   	curr_notice->event.create_time =clock;
	   	curr_notice->event.occur_time =End_time;
	   	curr_notice->right = NULL;
	   	curr_notice->left = NULL;
	   	sprintf(curr_notice->event.name, "END"); 
	   	schedule(curr_notice);
    } 
}



void arrival(nodePtr node_event){
	/* manage arrival event */
	
	nodePtr next_job;
	
	/* Update statistics */
	nsys++;
	narr++;
	
	node_event->event.create_time =clock;
	node_event->event.arrival_time =clock;
	
	
	if (nsys == 1) {
		/* Process arrival at idle server */
		node_event->event.type = DEPARTURE;
		node_event->event.occur_time = clock+node_event->event.service_time;
		schedule(node_event);
	}
	else {
		/* Process arrival at busy server */
		node_event->event.type = NO_EVENT;
		node_event->event.occur_time = 0.0;
		enqueue(node_event,&Input_Queue);
		ndelayed++;
	}
	if (!feof(fp)) {
	   /* get new pair fromn trace */
	   arrival_t = clock + GetInterArrival(fp);
	   service_t = GetService(fp);
	
	   /* schedule new arrival */
	   next_job = get_new_node();
	   next_job->event.type = ARRIVAL;
	   next_job->event.create_time =clock;
	   next_job->event.occur_time = arrival_t;
	   next_job->event.service_time = service_t;
	   next_job->event.arrival_time = arrival_t;
	   next_job->right = NULL;
	   next_job->left = NULL;
	   sprintf(next_job->event.name, "J%d", (job_number++));
	   schedule(next_job);
    }
}

void departure(nodePtr node_event){
	/* manage departure event */
	nodePtr next_job;
	
	/* Update statistics */
	nsys--;
	ncom++;
	
	if (nsys > 0) {
		/* Process departure from a server with a queue*/
		next_job = dequeue(&Input_Queue);
		next_job->event.type = DEPARTURE;
		next_job->event.occur_time = clock + next_job->event.service_time;
		schedule(next_job);
	}
	return_node(node_event);
	return_number++;
}

void end(nodePtr node_event){
	/* manage END event */
	halt = true;
	/* Observation period ends with the moment of last deprture */
	Stop = lasttime;
	return_node(node_event);
	return_number++;
}

/* =========================== */
   double GetInterArrival(FILE *fp)                 /* read an arrival time */
/* =========================== */
{ 
  double a;

  fscanf(fp, "%lf", &a);
  return (a);
}

/* =========================== */
   double GetService(FILE *fp)                 /* read a service time */
/* =========================== */
{ 
  double s;

  fscanf(fp, "%lf\n", &s);
  return (s);
}


void print_results(){
		
	printf("\n-------------------------------------------------------------------------");
	printf("\nSIMULATION RUN STATISTICS");
    printf("\n-------------------------------------------------------------------------");
    printf("\nNumber of new generated nodes         = %d",node_number);
    printf("\nNumber of processed events            = %d",event_counter);
	printf("\nLength of Observation Period          = %10.6f", ObservPeriod);
	printf("\nLast Future Event List:");
	print_FEL();
	printf("\nLast Server_Queue:");
	print_list(&Input_Queue);
	printf("\n-------------------------------------------------------------------------");	
   
}

void report(){
	
    ObservPeriod = Stop - Start;

	print_results();
	
	destroy_list(FEL.Head);
	destroy_list(Input_Queue.Head);
	destroy_list(AL.Head);
}

