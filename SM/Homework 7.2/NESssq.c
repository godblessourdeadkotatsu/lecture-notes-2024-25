/* Definition of the type used to specify the pointer to a node of a list or queue */
typedef struct node* nodePtr;

/* Definition of the Event Notice - typical fields to contain event’s attributes */
typedef struct {
    int type;
    char name[256];
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

void initialize() {
    /* Definition and Initialization of Useful Variables */
    clock = 0;
    halt = false;

    /* Definition and Initialization of Future Event List (FEL) and additional structures */
    FEL = NULL;  // Future Event List initialized to NULL
    Queue = NULL;  // Queue initialized to NULL (for managing events)

    /* First Arrival Time is 0 */
    double arrival_t = 0;  // Set the first arrival time to 0

    /* Extract the first Service Time as an Exponential Random Variable */
    double service_t = get_exponential_service_time();  // Assume this function generates the service time

    /* Initialize Event notice of first arrival and Schedule first event */
    nodePtr event_notice = get_new_node();  // Create a new node for the event
    event_notice->event.type = ARRIVAL;  // Set the type to ARRIVAL
    event_notice->event.occur_time = arrival_t;  // Set the event's occurrence time
    event_notice->event.arrival_time = arrival_t;  // Set the arrival time
    event_notice->event.service_time = service_t;  // Set the service time
    schedule(event_notice);  // Schedule this event in the Future Event List

    /* Initialize Event notice of End Simulation and Schedule last event */
    nodePtr end_event_notice = get_new_node();  // Create a new node for the end event
    end_event_notice->event.type = END;  // Set the type to END
    end_event_notice->event.occur_time = End_time;  // Set the end time (assuming you have this predefined)
    schedule(end_event_notice);  // Schedule the end simulation event in the Future Event List
}