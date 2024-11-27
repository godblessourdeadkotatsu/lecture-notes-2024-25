#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10 //max number of machines
#define HETA_ARRIVAL 3000 // Example lambda value for arrivals
#define HETA_SHORT 40   // Example lambda value for short station
#define HETA_LONG 960    // Example lambda value for long station
#define MAX_EVENTS 1000
#define BETA 0.2

/*
Now we prepare all we need to simulate these events.
First, we tackle the event list:
We define a structure Event that holds the type of event (arrival, departure
from long station or departure from short station) and the time of occurrence.
*/

typedef struct Event {
    int machine; // this 
    char type; // 'A' for Arrival, 'S' for Departure (Short Station), 'L' for
             // Departure (Long Station)
    float timestamp; // The time the event occurs
} Event;

int verbose = 0; // the cute logs are off by default.
int n=N;
double cycle_waiting_time_short_station = 0;
double cycle_waiting_time_long_station = 0;
double total_waiting_time_short = 0;
double total_waiting_time_long = 0;
int cycle_count = 0;
int L_event_count = 0;
double avg_waiting_time_long = 0;

float heta_arrival = HETA_ARRIVAL;
float heta_short = HETA_SHORT;
float heta_long = HETA_LONG;
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
float exponential_random(float lambda);
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

  // Create and add the first arrival event (Example)
  Event first_arrival;
  first_arrival.type = 'A';      // Arrival event
  first_arrival.timestamp = 0.0; // Starting time
  add_event(first_arrival);

  // now the main loop. we need to iterate for each element of the event list,
  // which will grow forever.
  for (int i = 0; i < MAX_EVENTS; i++) {
    Event current_event = event_list[i];
    process_event(current_event, i);
  }

  // Capture the end time
  clock_t end_time = clock();

  // Calculate the time taken and print it
  double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

  // Now calculate our point estimator
  avg_waiting_time_long=total_waiting_time_long/L_event_count;

  printf("\n_______________________________________\n");
  printf("Simulation complete! ");
  printf("Execution time: %f seconds\n", time_taken);
  printf("Total waiting time in short station: %f\n", total_waiting_time_short);
  printf("Total waiting time in long station: %f\n", total_waiting_time_long);
  printf("Total cycles: %d.\n", cycle_count);
  printf("Average waiting time for the long station: %f",avg_waiting_time_long);

  // Cleanup allocated memory
  cleanup_event_list();

  return 0;
}

float exponential_random(float heta) {
  return -heta*logf(1- (float)rand() / RAND_MAX);
}

void add_event(Event new_event) {
  // Check if resizing is needed
  if (event_count == event_capacity) {
    event_capacity =
        (event_capacity == 0) ? 10 : event_capacity * 2; // Double capacity
    Event *temp_list = realloc(event_list, event_capacity * sizeof(Event));
    if (!temp_list) {
      fprintf(stderr, "Memory allocation failed!\n");
      exit(EXIT_FAILURE);
    }
    // using a temporary pointer allows us to use the list even if the realloc
    // fails.
    event_list = temp_list;
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
    Of course, this means that each cycle j will have \nu_j=1 event count, since we always regenerate at the first L event found.
    This will be used to compute the point estimate of the waiting time.
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
  verbose ? printf("Now processing: %c event, number %d in the event list, at "
                   "time %f.\n",
                   current_event.type, pos, current_event.timestamp)
          : 0;
  if (current_event.type == 'A') {
    // now we create and insert the next arrival
    if (n>0) {
    Event next_arrival;
    next_arrival.type = 'A';
    next_arrival.timestamp =
        current_event.timestamp + exponential_random(heta_arrival);
    add_event(next_arrival);
    n--;
    verbose ? printf("Added the next arrival event %c, at time %f\n",
                     next_arrival.type, next_arrival.timestamp)
            : 0;
            }

    // we must check the case in which the queue is empty and in which it isn't.
    // remember, we start from the "pos" position an empty queue means that
    // there are no departure events ahead of our current event
    float last_departure_timestamp = -1.0;
    // if we do not find any 'S' events out timestamp will remain -1.0 and
    // that's how we will know that there are no S events scheduled ahead of our
    // current event
    for (size_t i = pos; i < event_count; i++) {
      if (event_list[i].type == 'S') {
        last_departure_timestamp = event_list[i].timestamp;
      }
    }
    // now we create and insert the departure of the current event
    Event current_departure;
    current_departure.type = 'S';
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
    verbose ? printf("Added the departure event %c (corresponding to our "
                     "current event %d) at time %f\n",
                     current_departure.type, pos, current_departure.timestamp)
            : 0;

    // compute the waiting time for this current event (which IS AN ARRIVAL)
    cycle_waiting_time_short_station +=
        current_departure.timestamp - current_event.timestamp;
  } else if (current_event.type == 'S') {
    // an empty queue means that there are no departure events ahead of our
    // current event
    float last_departure_timestamp = -1.0; // sentinel reset!
    // first, we decide whether this event is going to the long repair station
    // or nah
    float coin_flip = (rand() / (float)RAND_MAX);
    if (coin_flip <= beta) {
      verbose ? printf("Bad luck! you get a long repair! ") : 0;
      // go to the long station
      Event current_departure_long;
      current_departure_long.type = 'L';
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
            exponential_random(heta_long); // empty queue
        verbose
            ? printf(
                  "But at least there was no one in queue for the long station")
            : 0;
      } else {
        current_departure_long.timestamp =
            last_departure_timestamp +
            exponential_random(heta_long); // someone in the queue
      }
      // insert the event
      add_event(current_departure_long);
      verbose
          ? printf("\nAdded event %c at time %f\n", current_departure_long.type,
                   current_departure_long.timestamp)
          : 0;

      /*
      now we have scheduled the departure from the long station;
      we need to calculate the waiting time for this machine
      to exit the long station
      */
      cycle_waiting_time_long_station +=
          current_departure_long.timestamp - current_event.timestamp;
    } else {
      n++; //the machine goes back to the source
    }
  } else if (current_event.type == 'L') {
    n++; //the machine goes back to the source
    verbose ? printf("Found a L event. Nothing to do, move on!\n") : 0;
    // here we only need to collect statistics
  }
  verbose
      ? printf("Done processing event %c, number %d in the list, at time %f. In the source there are %d machines left.\n",
               current_event.type, pos, current_event.timestamp,
               n)
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

double ComputeConfidenceIntervals(double avg_waiting_time_long) {
  double s2_A = 1/(cycle_count-1);
  return 1.96;
}
