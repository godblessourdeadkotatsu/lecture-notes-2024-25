#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef enum {ARRIVAL, DEPARTURE_SHORT, DEPARTURE_LONG} EventType;

typedef struct {
    double time;          // Time of the event
    EventType type;       // Type of event (Arrival, Short station departure, Long station departure)
    double service_time;  // The service time of the current customer
} Event;

// Function declarations: this is
int RegPoint(int event_type);
void CollectRegStatistics(double waiting_time_cycle, double *total_waiting_time, int *cycle_count);
void ResetMeasures(double *waiting_time_cycle);
double DrawExponential(double lambdainv);
void schedule_event(int type, double time);
void add_event(Event new_event);
void process_events();


int RegPoint(int event_type) {
// Regeneration point occurs at a DEPARTURE event
// Returns 1 (true) for regeneration point, 0 (false) otherwise
return event_type == DEPARTURE;
}

void CollectRegStatistics(double waiting_time_cycle, double *total_waiting_time, int *cycle_count) {
*total_waiting_time += waiting_time_cycle;  // Accumulate waiting time
(*cycle_count)++;  // Increment the cycle count
}

void ResetMeasures(double *waiting_time_cycle) {
*waiting_time_cycle = 0.0;  // Reset the cycle-specific waiting time
}

double DrawExponential(double lambdainv) {
}

void add_event(Event new_event) {
    // Check if resizing is needed
    if (event_count == event_capacity) {
        event_capacity = (event_capacity == 0) ? 10 : event_capacity * 2; // Double capacity
        event_list = realloc(event_list, event_capacity * sizeof(Event));
        if (!event_list) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(EXIT_FAILURE);
        }
    }

    // Add the new event
    event_list[event_count++] = new_event;

    // Sort the array (using insertion sort logic)
    int i = event_count - 1;
    while (i > 0 && event_list[i - 1].time > event_list[i].time) {
        Event temp = event_list[i];
        event_list[i] = event_list[i - 1];
        event_list[i - 1] = temp;
        i--;
    }
}

void process_event(Event *event, Event *event_queue, int *queue_size, double *total_short_waiting_time, double *total_long_waiting_time) {
    switch (event->type) {
        case ARRIVAL: {
            // Handle arrival: Draw service time, schedule short departure
            double short_service_time = draw_service_time(SHORT); // Your service time drawing function here
            Event short_departure = {event->time + short_service_time, DEPARTURE_SHORT, short_service_time};
            event_queue[*queue_size] = short_departure;
            (*queue_size)++;

            // Handle whether the customer will go to the long station (80-20 decision)
            if (rand() % 100 < 20) {  // 20% chance
                double long_service_time = draw_service_time(LONG); // Service time at long station
                Event long_departure = {short_departure.time + long_service_time, DEPARTURE_LONG, long_service_time};
                event_queue[*queue_size] = long_departure;
                (*queue_size)++;
            }
            break;
        }
        case DEPARTURE_SHORT: {
            // Update total short station waiting time
            break;
        }
        case DEPARTURE_LONG: {
            // Update total long station waiting time
            break;
        }
        default:
            // Handle other event types if necessary
            break;
    }
}