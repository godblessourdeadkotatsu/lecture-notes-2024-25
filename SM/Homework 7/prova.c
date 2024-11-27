#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct Event {
    char type;          // 'A' for Arrival, 'S' for Departure (Short Station), 'L' for Departure (Long Station)
    float timestamp;    // The time the event occurs
} Event;

int RegPoint(Event current_event);

void main() {
    Event prova;
    prova.type='A';
    prova.timestamp=20;
    printf("%d",RegPoint(prova));
}

int RegPoint(Event current_event) {
    // Regeneration point occurs at a S or L event
    // Returns 1 (true) for regeneration point, 0 (false) otherwise
    return (current_event.type=='S'||current_event.type=='L');
}