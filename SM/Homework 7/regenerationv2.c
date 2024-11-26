#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/*
Now we prepare all we need to simulate these events. 
First, we tackle the event list:
We define a structure Event that holds the type of event (arrival, departure from long station or departure from short station) and the time of occurrence.
*/

typedef struct Event {
    char type;          // 'A' for Arrival, 'S' for Departure (Short Station), 'L' for Departure (Long Station)
    float timestamp;    // The time the event occurs
} Event;

/*
Now we create a pointer to the even list (that will be an array of the type Event).
Why a pointer? Because the array will change size becoming larger or smaller as soon as we process events.
*/

Event* event_list = NULL;    // Pointer to the array of events
size_t event_count = 0;      // Number of events currently in the list. This will grow as we add more events.
size_t event_capacity = 0;   // This is how much memory we have allocated for this array. When the list is full we need ro realloc an expanded version of this array.

float exponential_random() {
    return -logf((float) rand() / RAND_MAX);
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

    // Add the new event to the list (temporarily at the end)
    event_list[event_count] = new_event;
    event_count++;

    // Sort the event list (insertion sort)
    size_t i = event_count - 1;
    while (i > 0 && event_list[i - 1].timestamp > event_list[i].timestamp) {
        Event temp = event_list[i];
        event_list[i] = event_list[i - 1];
        event_list[i - 1] = temp;
        i--;
    }
}

void process_event(Event current_event, int pos) {
    //non sono convinto di sta parte

    /*
    if (event_count == 0) return;  // No events to process, exit early

    // Get the event with the smallest timestamp (first in the sorted list)
    Event current_event = event_list[0];

    // Remove the first event from the list
    for (size_t i = 0; i < event_count - 1; i++) {
        event_list[i] = event_list[i + 1];
    }
    event_count--;
    */

    // Process the event depending on its type
    if (current_event.type == 'A') {

        // now we create and insert the next arrival
        Event next_arrival;
        next_arrival.timestamp=current_event.timestamp+exponential_random(lambda_arrival);
        next_arrival.type ='A';
        add_event(next_arrival);

    
        // we must check the case in which the queue is empty and in which it isn't. remember, we start from the "pos" position
        // an empty queue means that there are no departure events ahead of our current event
        float last_departure_timestamp = -1.0;
        // if we do not find any 'S' events out timestamp will remain -1.0 and that's how we will know that there are no S events scheduled ahead of our current event
        for (size_t i = pos; i < event_count; i++) {
            if (event_list[i].type == 'S') {
                last_departure_timestamp = event_list[i].timestamp;
            }
        }
        // now we create and insert the departure of the current event
        Event current_departure;
        current_departure.type='S'
        if (last_departure_timestamp==-1.0){ 
            current_departure.timestamp=current_event.timestamp+exponential_random(lambda_short); //empty queue
        } else {
            current_departure.timestamp=last_departure_timestamp+exponential_random(lambda_short); //someone in the queue
        }
        //insert the fucking event
        add_event(current_departure);

    } else if (current_event.type == 'S') {
        //first, we decide whether this event is going to the long repair station or nah
        float coin_flip = (rand() / (float)RAND_MAX);
        if (coin_flip <=0.2){
            Event current_departure_long;
            current_departure_long.type='L';
            //do the same thing as before
            for (size_t i = pos; i < event_count; i++) {
            if (event_list[i].type == 'L') {
                last_departure_timestamp = event_list[i].timestamp;
            }
            }
            //we got our sentinel
            if (last_departure_timestamp==-1.0){ 
            current_departure_long.timestamp=current_event.timestamp+exponential_random(lambda_long); //empty queue
            } else {
            current_departure_long.timestamp=last_departure_timestamp+exponential_random(lambda_long); //someone in the queue
        }
        }

    } else if (current_event.type == 'L') {
        process_departure_long(current_event);  // Handle long station departure
    }
}


/*cleanup allocated memory*/
void cleanup_event_list() {
    free(event_list);  // Release the allocated memory
    event_list = NULL; // Set pointer to NULL for safety
    event_count = 0;
    event_capacity = 0;
}
