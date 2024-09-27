/* $Id: ssq2.c 55 2005-09-13 22:29:52Z asminer $ */
/* -------------------------------------------------------------------------
 * This program simulates a single-server FIFO service node using arrival
 * times and service times read from a text file.  The server is assumed
 * to be idle when the first job arrives.  All jobs are processed completely
 * so that the server is again idle at the end of the simulation.   The
 * output statistics are the average interarrival time, average service
 * time, the average delay in the queue, and the average waiting in the service 
 * node. 
 *
 * Name              : ssq1.c  (Single Server Queue, version 1)
 * Authors           : Steve Park & Dave Geyer
 * Language          : ANSI C
 * Latest Revision   : 9-01-98
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
  double first_arrival;
  double first_service;
  double max_delay = 0;
  long customers_at_400 =0;
  long delayed = 0;
  long   index     = 0;                        /* job index            */
  double inter_arrival;                        /* inter_arrival time   */
  double arrival;                              /* arrival time         */
  double delay;                                /* delay in queue       */
  double service;                              /* service time         */
  double wait;                                 /* delay + service      */
  double departure;                            /* departure time       */
  struct {                                     /* sum of ...           */
    double delay;                              /*   delay times        */
    double wait;                               /*   wait times         */
    double service;                            /*   service times      */
    double interarrival;                       /*   interarrival times */
  } sum = {0.0, 0.0, 0.0};

  fp = fopen(FILENAME, "r");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open input file %s\n", FILENAME);
    return (1);
  }

first_arrival=GetInterArrival(fp);
first_service=GetService(fp);

fseek(fp, 0, SEEK_SET);  // Go back to the start of the file
arrival = START;

  while (!feof(fp)) {
    index++;
    inter_arrival = GetInterArrival(fp);
    arrival = arrival + inter_arrival;
    if (arrival < departure) 
      {delay      = departure - arrival;        /* delay in queue    */
      delayed++;}
    else 
      delay      = 0.0;                        /* no delay          */
    if (delay>max_delay)
      max_delay=delay;
    service      = GetService(fp);
    wait         = delay + service;
    departure    = arrival + wait;             /* time of departure */
    if (arrival<400 && departure>400)
    customers_at_400++;
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
  printf("   input rate ............ = %14.10f\n", index / sum.interarrival);
  printf("   service rate! ............ = %14.10f\n", index/sum.service);
  printf("   traffic intensity ............ = %14.10f\n", sum.wait / sum.interarrival);
  printf("   throughput ............ = %14.10f\n", index / (departure-(first_arrival+first_service)));
  printf("   utilization ............ = %14.10f\n", sum.service / (departure-(first_arrival+first_service)));
  printf("   max delay ............ = %14.10f\n", max_delay);
  printf("   %14.3f %% of customers experienced a delay\n", (double)delayed/index);
  printf("   At time 400 there were %ld customers in the server\n", customers_at_400);
  fclose(fp);
  return (0);
}
