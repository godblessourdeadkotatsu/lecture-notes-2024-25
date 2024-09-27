/* $Id: ssq2.c 55 2005-09-13 22:29:52Z asminer $ */
/* -------------------------------------------------------------------------
 * This program simulates a single-server FIFO service node using inter-arrival
 * times and service times read from a text file.  The server is assumed
 * to be idle when the first job arrives.  All jobs are processed completely
 * so that the server is again idle at the end of the simulation.   The
 * output statistics are the average interarrival time, average service
 * time, the average delay in the queue, and the average wait in the service 
 * node. 
 *
 * Name              : ssq2.c  (Single Server Queue, version 1)
 * Authors           : Steve Park & Dave Geyer
 * Language          : ANSI C
 * Latest Revision   : 9-01-98
 * Modified by       : Lorenzo Sala
 * Latest Revision   : 25-09-24
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>                              

#define FILENAME   "ssq1.dat"                  /* input data file */
#define START      0.0

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

/* ============== */
   int main(void)
/* ============== */
{
  FILE   *fp;                                  /* input data file      */
  double first_arrival;                         /*time of first arrival (we will need it lated)*/            
  double first_service;                         /*time of first service (we will need it lated)*/
  double max_delay = 0;                         /*initialize maximum delay*/
  long customers_at_400 =0;                     /*initialize customers at t=400 counter*/
  long delayed = 0;                             /*initialize number of customers that experience a delay*/
  long   index        = 0;                     /* job index            */
  double arrival;                              /* arrival time         */
  double last_arrival =  START;                /*  last arrival time   */
  double delay;                                /* delay in queue       */
  double service;                              /* service time         */
  double wait;                                 /* delay + service      */
  double departure = START;                    /* departure time       */
  struct {                                     /* sum of ...           */
    double delay;                              /*   delay times        */
    double wait;                               /*   wait times         */
    double service;                            /*   service times      */
    double interarrival;                       /*   interarrival times */
  } sum = {0.0, 0.0, 0.0, 0.0};

  fp = fopen(FILENAME, "r");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open input file %s\n", FILENAME);
    return (1);
  }

first_arrival=GetInterArrival(fp)-START;            /*get first arrival for later use*/
first_service=GetService(fp);                       /*get first service*/

fseek(fp, 0, SEEK_SET);  /*reset pointer pf*/

  while (!feof(fp)) {
    index++;
    arrival      = last_arrival + GetInterArrival(fp);
    last_arrival = arrival;
   if (arrival < departure) 
      {delay      = departure - arrival;        /* delay in queue    */
      delayed++;}                               /*add one to the number of people who experienced a delay*/
    else 
      {delay      = 0.0; }                       /* no delay */
  if (delay>max_delay)
      {max_delay=delay;}                        /*update max delay up to time index*/
    service      = GetService(fp);
    wait         = delay + service;
    departure    = arrival + wait;             /* time of departure */
    if (arrival<=400 && departure>400)   /*this counts specifically people inside the server at time 400*/
      customers_at_400++;                /*that is people who arrived before 400 and left after 400*/
    sum.delay   += delay;
    sum.wait    += wait;
    sum.service += service;
  }
  sum.interarrival = arrival - START;

  printf("\nfor %ld jobs\n", index);
  printf("   average interarrival time = %14.10f\n", sum.interarrival / index);
  printf("   average service time .... = %14.10f\n", sum.service / index);
  printf("   average delay ........... = %14.10f\n", sum.delay / index);
  printf("   average wait ............ = %14.10f\n", sum.wait / index);
  printf("   input rate ............ = %14.10f\n", index / sum.interarrival); /*from definition of input */
  printf("   service rate ............ = %14.10f\n", index/sum.service);
  printf("   traffic intensity ............ = %14.10f\n", sum.wait / sum.interarrival);
  printf("   throughput ............ = %14.10f\n", index / (departure-(first_arrival+first_service)));
  printf("   utilization ............ = %14.10f\n", sum.service / (departure-(first_arrival+first_service)));
  printf("   max delay ............ = %14.10f\n", max_delay);
  printf("   %14.3f %% of customers experienced a delay\n", (double)delayed/index)*100; /*just a proportion between two indexes*/
  printf("   At time t=400 there were %ld customers in the server\n", customers_at_400);
  /*this and the following statement follow little's law.
  This states that the average number of people present in the structure (considering people delayed and people serviced) 
  is equal to the input rate multiplied by the average wait time and delay time respectively.
  This should be much faster (and easier to program) than keeping track of each singular arrival and departure.*/
  printf("   average number of people present in the structure ............ = %14.10f\n", (index / sum.interarrival)*(sum.wait / index)); 
  printf("   average number of people waiting in the structure ............ = %14.10f\n", (index / sum.interarrival)*(sum.delay / index));
  fclose(fp);
  getchar ();
  return (0);
  
}
