#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10              // max number of machines
#define HETA_ARRIVAL 3000 // Expected value for arrivals
#define HETA_SHORT 40     // Expected value for short station
#define MAX_EVENTS 1000000 // Do NOT EVEN TRY to pass the verbose command with such a big event number  
#define BETA 0.2 // routing probability

/*
Now we prepare all we need to simulate these events.
First, we tackle the event list:
We define a structure Event that holds the type of event (arrival, departure
from long station or departure from short station) and the time of occurrence.
*/

typedef struct Event {
  int machine; // this will identify what machine we are talking about.
  char type;   // 'A' for Arrival, 'S' for Departure (Short Station), 'L' for
               // Departure (Long Station)
  double timestamp; // The time the event occurs
} Event;

int verbose = 0; // the cute logs are off by default.
int n = N;
double cycle_waiting_time_short_station = 0;
double cycle_waiting_time_long_station = 0;
double total_waiting_time_short = 0;
double total_waiting_time_long = 0;
int cycle_count = 0;
int L_event_count = 0;
double avg_waiting_time_long = 0;
double total_waiting_time_short_squared = 0;
double total_waiting_time_long_squared = 0; // these are needed to compute the variance 
double time_events_product_short = 0; // this is needed to compute the covariance
double time_events_product_long = 0;


double heta_arrival = HETA_ARRIVAL;
double heta_short = HETA_SHORT;
double alpha[] = {0.95,0.05};
double mu[] = {10, 19010};
double beta = BETA;

/*
Now we create a pointer to the even list (that will be an array of the type
Event). Why a pointer? Because the array will change size becoming larger or
smaller as soon as we process events.
*/

Event *event_list = NULL; // Pointer to the array of events
size_t event_count = 0;   // Number of events currently in the list. This will
                          // grow as we add more events.
size_t event_capacity =
    0; // This is how much memory we have allocated for this array. When the
       // list is full we need ro realloc an expanded version of this array.

// function declaration
double uniform_random();
double exponential_random(double lambda);
double hyperexponential_random(double* alpha, double* mu);
void add_event(Event new_event);
void process_event(Event current_event, int pos);
void cleanup_event_list();
int RegPoint(Event current_event);
void CollectRegStatistics(double *cycle_waiting_time_short_station,
                          double *cycle_waiting_time_long_station,
                          double *total_waiting_time_short,
                          double *total_waiting_time_long, int *cycle_count);
void ResetMeasures(double *cycle_waiting_time_long_station,
                   double *cycle_waiting_time_short_station);
double ComputeConfidenceIntervals(double avg_waiting_time_long, double time_events_product_long, double L_event_count, double total_waiting_time_long_squared, int cycle_count);

int main(int argc, char *argv[]) {

  clock_t start_time = clock(); // start the stopwatch

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] == 'v') {
      verbose = 1; // Enable verbose logging
      break;       // No need to check further once we find the flag
    }
  }
  // MORE INITIALIZATION
  srand(time(NULL));                       // Seed the random number generator
  event_list = malloc(10 * sizeof(Event)); // Initial allocation for 10 events
  event_capacity = 10;
  if (!event_list) {
    fprintf(stderr, "Memory allocation failed!\n");
    exit(EXIT_FAILURE);
  }

  // Create and add the first arrival event for each machine
  for (int i = 1; i <= n; i++) {
    Event first_event;
    first_event.machine = i;
    first_event.type = 'A';
    first_event.timestamp = exponential_random(heta_arrival);
    add_event(first_event);
  }

  // now we have all the first arrivals for each machine. What we need to do is
  // scale all of these events to start from 0 
  double delta = event_list[0].timestamp;
  event_list[0].timestamp = 0;
  for (int i = 1; i < n; i++) {
    int temp = event_list[i].timestamp;
    event_list[i].timestamp = temp - delta;
  }

  // now the main loop. we need to iterate for each element of the event list,
  // which will grow forever.
  for (int i = 0; i < MAX_EVENTS; i++) {
    Event current_event = event_list[i];
    process_event(current_event, i);
    if (isinf(current_event.timestamp)) {
      printf("Timestamp reached infinity. Stopping the loop. Something is "
             "wrong.\n");
      break; // Exit the loop 
    }
  }

  // Capture the end time
  clock_t end_time = clock();

  // Calculate the time taken and print it
  double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

  // Now calculate our point estimator
  avg_waiting_time_long = total_waiting_time_long / L_event_count;

  // Calculate the sample variance (with an alternative formula)
  double error = ComputeConfidenceIntervals(avg_waiting_time_long, time_events_product_long, L_event_count, total_waiting_time_long_squared, cycle_count);

  double error_percentage =  error/avg_waiting_time_long;
  

  printf("\n_______________________________________\n");
  printf("Simulation complete! ");
  printf("Execution time: %f seconds\n", time_taken);
  printf("Total waiting time in short station: %f\n", total_waiting_time_short);
  printf("Total waiting time in long station: %f\n", total_waiting_time_long);
  printf("Total cycles: %d.\n", cycle_count);
  printf("Average waiting time for the long station: %f\n",
         avg_waiting_time_long);
  printf("Confidence interval at 10%%: (%f,%f).\n", avg_waiting_time_long-error,avg_waiting_time_long+error);
  printf("The error is the %f%% of the point estimate.\n", error_percentage);

  // Cleanup allocated memory
  cleanup_event_list();

  return 0;
}

/*
We need to scale down these values so that we do not go in overflow. We are only
interested in the relative order, anyway
*/

/*
double exponential_random(double heta) {
  double unif = (double)rand() / RAND_MAX;
  if ((1 - unif) < 1e-20) {
    unif = 1e-20; // this will give us a limit in the case in which the argument
                  // of the logarithm is too close to 0 (and thus exploding)
  }
  double exp = (-heta * logf(1 - unif));
  // double pick = 1- ((double)rand() / RAND_MAX);
  verbose ? printf("This time I extracted %f!\n", exp) : 0;
  return exp;
}


double uniform_random() {
    return (double)rand() / RAND_MAX;  // Generates a uniform random number in [0,1]
}
*/

double exponential_random(double lambda) {
    double u = (double)rand() / RAND_MAX;
    if ((1 - u) < 1e-20) {
    u = 1e-20; // this will give us a limit in the case in which the argument
                  // of the logarithm is too close to 0 (and thus exploding)
  }
    return -log(1 - u) * lambda;  // Inverse transform method
}

double hyperexponential_random(double* p, double* lambda) {
    double u = (double)rand() / RAND_MAX;
    double cumulative_probability = 0.0;

    // Choose the component
    for (int i = 0; i < 2; i++) {
        cumulative_probability += p[i];
        if (u < cumulative_probability) {
            return exponential_random(lambda[i]);  // Sample from chosen exponential
        }
    }
}

void add_event(Event new_event) {
  // Check if resizing is needed
  if (event_count == event_capacity) {
    size_t old_capacity = event_capacity;
    event_capacity = (event_capacity == 0) ? 10 : event_capacity * 2;
    Event *temp_list = realloc(event_list, event_capacity * sizeof(Event));
    if (!temp_list) {
      fprintf(stderr, "Memory allocation failed!\n");
      exit(EXIT_FAILURE);
    }
    event_list = temp_list;

    // Initialize the newly allocated memory
    for (size_t i = old_capacity; i < event_capacity; i++) {
      event_list[i].machine = 0;
      event_list[i].type = '\0';
      event_list[i].timestamp = 0.0f;
    }
  }

  // Add the new event to the list (temporarily at the end)
  event_count++;
  event_list[event_count - 1] = new_event;

  // Sort the event list (insertion sort)
  size_t i = event_count - 1;
  while (i > 0 && event_list[i - 1].timestamp > event_list[i].timestamp) {
    Event temp = event_list[i];
    event_list[i] = event_list[i - 1];
    event_list[i - 1] = temp;
    i--;
  }
}

/*
Now we prepare the engine of our simulation.
*/

void process_event(Event current_event, int pos) {
  // start by doing the task of verifying whether the event is a regeneration
  // point if it is, then collect the statistics.
  if (RegPoint(current_event)) {
    /* we accumulate the number of L events in a global accumulator.
    Of course, this means that each cycle j will have \nu_j=1 event count, since
    we always regenerate at the first L event found. This will be used to
    compute the point estimate of the waiting time.
    */
    L_event_count++;
    CollectRegStatistics(&cycle_waiting_time_short_station,
                         &cycle_waiting_time_long_station,
                         &total_waiting_time_short, &total_waiting_time_long,
                         &cycle_count); // collect statistics
    ResetMeasures(&cycle_waiting_time_long_station,
                  &cycle_waiting_time_short_station);

    verbose ? printf("I found event of type %c, which is a regeneration point. "
                     "I cleaned statistics and accumulated what I had to "
                     "accumulate.\n\n",
                     current_event.type)
            : 0;
  }

  // Process the event depending on its type
  verbose ? printf("Now processing: %c event of machine %d, number %d in the "
                   "event list, at "
                   "time %f.\n",
                   current_event.type, current_event.machine, pos,
                   current_event.timestamp)
          : 0;
  if (current_event.type == 'A') {
    // we compute when this machine will depart.
    // we must check the case in which the queue is empty and in which it isn't.
    // remember, we start from the "pos" position an empty queue means that
    // there are no departure events ahead of our current event.
    // We don't care about what machine is departing, the order will be
    // scrambled anyway
    double last_departure_timestamp = -1.0;
    // if we do not find any 'S' events out timestamp will remain -1.0 and
    // that's how we will know that there are no S events scheduled ahead of our
    // current event
    for (size_t i = pos; i < event_count; i++) {
      if (event_list[i].type == 'S') {
        last_departure_timestamp = event_list[i].timestamp;
      }
    }
    // now we create and insert the departure of the current event. This event
    // will always be about the same machine of the
    Event current_departure;
    current_departure.type = 'S';
    current_departure.machine = current_event.machine;
    if (last_departure_timestamp == -1.0) {
      current_departure.timestamp =
          current_event.timestamp +
          exponential_random(heta_short); // empty queue
      verbose ? printf("Wow, no one in queue! ") : 0;
    } else {
      current_departure.timestamp =
          last_departure_timestamp +
          exponential_random(heta_short); // someone in the queue
    }
    // insert the event
    add_event(current_departure);
    verbose ? printf("Added the departure event %c of the machine "
                     "%d(corresponding to our "
                     "current event %d) at time %f\n",
                     current_departure.type, current_departure.machine, pos,
                     current_departure.timestamp)
            : 0;

    /* compute the waiting time for this current event (which IS AN ARRIVAL, but we know its departure now!).
    we also need to compute the square and accumulate it in another variable.
    we will need it to compute the sample variance!  */
    double waiting_time_short = current_departure.timestamp - current_event.timestamp;
    double waiting_time_short_squared = pow(waiting_time_short, 2);
    cycle_waiting_time_short_station += waiting_time_short;
    total_waiting_time_short_squared += waiting_time_short_squared;
    time_events_product_short += waiting_time_short*1;
  } else if (current_event.type == 'S') {

    /*
    Things get trickier: we have two scenarios when processing an S-event.
    If we do not go to the L station then our machine comes back to the pool and
    we can schedule its next departure. Otherwise... we need to send it to long
    repair, where something similar will happen.
    */

    double last_departure_timestamp = -1.0; // sentinel reset!
    // first, we decide whether this event is going to the long repair station
    // or not
    double coin_flip = (rand() / (double)RAND_MAX);
    if (coin_flip <= beta) {
      verbose ? printf("Bad luck, machine %d! You get a long repair! ",
                       current_event.machine)
              : 0;
      // go to the long station and schedule the departure event
      Event current_departure_long;
      current_departure_long.type = 'L';
      current_departure_long.machine = current_event.machine;
      // do the same thing as before
      for (size_t i = pos; i < event_count; i++) {
        if (event_list[i].type == 'L') {
          last_departure_timestamp = event_list[i].timestamp;
        }
      }
      // we got our sentinel
      if (last_departure_timestamp == -1.0) {
        current_departure_long.timestamp =
            current_event.timestamp +
            hyperexponential_random(alpha,mu); // empty queue
        verbose
            ? printf(
                  "But at least there was no one in queue for the long station")
            : 0;
      } else {
        current_departure_long.timestamp =
            last_departure_timestamp +
            hyperexponential_random(alpha,mu); // someone in the queue
      }
      // insert the event
      add_event(current_departure_long);
      verbose
          ? printf("\nAdded departure event %c for machine %d at time %f\n",
                   current_departure_long.type, current_departure_long.machine,
                   current_departure_long.timestamp)
          : 0;

      /*
      now we have scheduled the departure from the long station;
      we are now to calculate the waiting time for this machine
      to exit the long station
      */
      double waiting_time_long =  current_departure_long.timestamp - current_event.timestamp;
      double waiting_time_long_squared = pow(waiting_time_long,2);
      cycle_waiting_time_long_station += waiting_time_long;
      total_waiting_time_long_squared += waiting_time_long_squared;
      time_events_product_long += waiting_time_long * 1;
    } else {
      // the machine goes back to the source and we schedule its next arrival to
      // the short station.
      Event next_arrival_for_this_machine;
      next_arrival_for_this_machine.type = 'A';
      next_arrival_for_this_machine.machine = current_event.machine;
      next_arrival_for_this_machine.timestamp =
          current_event.timestamp + exponential_random(heta_arrival);
      add_event(next_arrival_for_this_machine);
      verbose ? printf("The machine %d gets back to the source. It will break "
                       "again at time %f\n",
                       next_arrival_for_this_machine.machine,
                       next_arrival_for_this_machine.timestamp)
              : 0;
    }
  } else if (current_event.type == 'L') {
    // the machine goes back to the source. Schedule its next arrival!
    Event next_arrival_after_long;
    next_arrival_after_long.type = 'A';
    next_arrival_after_long.machine = current_event.machine;
    next_arrival_after_long.timestamp =
        current_event.timestamp + exponential_random(heta_arrival);
    add_event(next_arrival_after_long);
    verbose ? printf("Found a L event. Scheduled next arrival for this machine "
                     "%d that will break again at time %f\n",
                     next_arrival_after_long.machine,
                     next_arrival_after_long.timestamp)
            : 0;
  }
  verbose ? printf("Done processing event %c regarding machine %d, number %d "
                   "in the list, at time %f. \n",
                   current_event.type, current_event.machine, pos,
                   current_event.timestamp)
          : 0;
  verbose ? printf("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n") : 0;
}

/*cleanup allocated memory*/
void cleanup_event_list() {
  free(event_list);  // Release the allocated memory
  event_list = NULL; // Set pointer to NULL for safety
  event_count = 0;
  event_capacity = 0;
}

int RegPoint(Event current_event) {
  // Regeneration point occurs at a S or L event
  // Returns 1 (true) for regeneration point, 0 (false) otherwise
  return (current_event.type == 'L');
}

void CollectRegStatistics(double *cycle_waiting_time_short_station,
                          double *cycle_waiting_time_long_station,
                          double *total_waiting_time_short,
                          double *total_waiting_time_long, int *cycle_count) {
  *total_waiting_time_short +=
      *cycle_waiting_time_short_station; // Accumulate waiting time for short
                                         // station
  *total_waiting_time_long +=
      *cycle_waiting_time_long_station; // Accumulate waiting time for short
                                        // station
  (*cycle_count)++;                     // Increment the cycle count
}

void ResetMeasures(double *cycle_waiting_time_long_station,
                   double *cycle_waiting_time_short_station) {
  *cycle_waiting_time_short_station =
      0.0; // Reset the cycle-specific waiting time
  *cycle_waiting_time_long_station =
      0.0; // Reset the cycle-specific waiting time
}

double ComputeConfidenceIntervals(double avg_waiting_time_long, double time_events_product_long, double L_event_count, double total_waiting_time_long_squared, int cycle_count) {
  // the derivations of the formulas used here are in the document 
  // we compute the variance with a slightly different formula (it is easier to implement)
  double s2_A= (total_waiting_time_long_squared-(cycle_count*pow(total_waiting_time_long,2))/(cycle_count-1));
  // we compute the covariance with a different formula (similar to the variance)
  double s_Anu = (time_events_product_long-cycle_count*avg_waiting_time_long*L_event_count)/(cycle_count-1);
  double s2_nu = (cycle_count-cycle_count*pow(L_event_count,2))/(cycle_count-1);
  double s2_Z = s2_A - 2*avg_waiting_time_long*s_Anu+pow(avg_waiting_time_long,2)*pow(s2_nu,2);
  double error = 1.96 * (sqrt(s2_Z))/(L_event_count*sqrt(cycle_count));
  return error;
}

