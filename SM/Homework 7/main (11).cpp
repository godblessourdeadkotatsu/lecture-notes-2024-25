/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <queue>
#include <boost/math/distributions/students_t.hpp>

#define N 10
#define LAMBDA_ARRIVAL 3000
#define LAMBDA_SHORT 40
#define LAMBDA_LONG 960
#define PROBABILITY_LONG_REPAIR 0.2
#define MIN_CYCLES 40
#define CONFIDENCE_LEVEL 1.96

#define ALPHA1 0.95
#define ALPHA2 0.05
#define MU1 10
#define MU2 19010

//α1 = 0.95, α2 = 0.05, µ1 = 10 min, and µ2 = 19010 min.

typedef enum { ARRIVAL, SHORT_REPAIR, LONG_REPAIR } EventType;

typedef struct Event {
    EventType type;
    double timestamp;
    int machine_id;
    struct Event* next;
} Event;

int machines_available = N;
double total_waiting_time_long = 0;
double cycle_waiting_time_long = 0;
int cycle_count = 0;
int long_repair_event_count = 0;
Event* event_queue = NULL;

std::vector<double> long_waiting_times;
std::vector<double> A_values;
std::vector<double> nu_values;

double regeneration_start_time = 0.0;
std::queue<double> long_repair_queue;
bool long_repair_busy = false;

double generate_exponential_random(double lambda) {
    double u = (double)rand() / RAND_MAX;
    return -lambda * log(1 - u);
}

double generate_hyperexponential_random(double lambda, double alpha) {
    double u = (double)rand() / RAND_MAX;
    return -lambda * log(1 - u) * alpha;
}


double generate_hyperexponential_random() {
    double u = (double)rand() / RAND_MAX;
    if (u <= ALPHA1) {
        return generate_exponential_random(MU1);
    } else {
        return generate_exponential_random(MU2);
    }
}

void schedule_event(EventType type, double timestamp, int machine_id) {
    Event* new_event = (Event*)malloc(sizeof(Event));
    new_event->type = type;
    new_event->timestamp = timestamp;
    new_event->machine_id = machine_id;
    new_event->next = NULL;

    if (event_queue == NULL || event_queue->timestamp > new_event->timestamp) {
        new_event->next = event_queue;
        event_queue = new_event;
    } else {
        Event* current = event_queue;
        while (current->next != NULL && current->next->timestamp < new_event->timestamp) {
            current = current->next;
        }
        new_event->next = current->next;
        current->next = new_event;
    }
}

Event* pop_event() {
    if (event_queue == NULL) {
        return NULL;
    }
    Event* event = event_queue;
    event_queue = event_queue->next;
    return event;
}

int is_regeneration_point(Event* event) {
    return event->type == LONG_REPAIR;
}

void collect_regeneration_statistics(double event_timestamp) {
    total_waiting_time_long += cycle_waiting_time_long;
    long_waiting_times.push_back(cycle_waiting_time_long);
    A_values.push_back(cycle_waiting_time_long);
    nu_values.push_back(event_timestamp - regeneration_start_time);
    cycle_count++;
}

void reset_cycle_measures(double event_timestamp) {
    regeneration_start_time = event_timestamp;
    cycle_waiting_time_long = 0;
}

void process_event(Event* event) {
    if (is_regeneration_point(event)) {
        long_repair_event_count++;
        collect_regeneration_statistics(event->timestamp);
        reset_cycle_measures(event->timestamp);
    }

    switch (event->type) {
        case ARRIVAL: {
            // Schedule next failure for this specific machine
            double next_arrival_time = event->timestamp + generate_exponential_random(LAMBDA_ARRIVAL);
            schedule_event(ARRIVAL, next_arrival_time, event->machine_id);
            
            machines_available--;
            double departure_time = event->timestamp + generate_hyperexponential_random(MU1, ALPHA1);
            //double departure_time = event->timestamp + ALPHA1 * generate_exponential_random(MU1);
            schedule_event(SHORT_REPAIR, departure_time, event->machine_id);
            break;
        }
        case SHORT_REPAIR: {
            if (((double)rand() / RAND_MAX) <= PROBABILITY_LONG_REPAIR) {
                if (!long_repair_busy) {
                    long_repair_busy = true;
                    double long_repair_time = event->timestamp + generate_hyperexponential_random(MU2, ALPHA2);
                    //double long_repair_time = event->timestamp + ALPHA2 * generate_exponential_random(MU2);
                    schedule_event(LONG_REPAIR, long_repair_time, event->machine_id);
                    cycle_waiting_time_long += long_repair_time - event->timestamp;
                } else {
                    long_repair_queue.push(event->timestamp);
                }
            } else {
                machines_available++;
            }
            break;
        }
        case LONG_REPAIR: {
            machines_available++;
            if (!long_repair_queue.empty()) {
                double next_queue_time = long_repair_queue.front();
                long_repair_queue.pop();
                double long_repair_time = event->timestamp + generate_hyperexponential_random(MU2, ALPHA2);
                //double long_repair_time = event->timestamp + ALPHA2 * generate_exponential_random(MU2);
                schedule_event(LONG_REPAIR, long_repair_time, event->machine_id);
                cycle_waiting_time_long += (event->timestamp - next_queue_time);
            } else {
                long_repair_busy = false;
            }
            break;
        }
    }
}


double compute_t_value(int degrees_of_freedom) {
    double alpha = 0.05;
    boost::math::students_t dist(degrees_of_freedom);
    return boost::math::quantile(dist, 1 - alpha / 2);
}

bool decide_to_stop() {
    if (cycle_count <= MIN_CYCLES) {
        return false;
    }

    double S_A = 0.0, S_v = 0.0, S_AA = 0.0, S_vv = 0.0, S_Av = 0.0;
    for (int j = 0; j < cycle_count; ++j) {
        S_A += A_values[j];
        S_v += nu_values[j];
        S_AA += A_values[j] * A_values[j];
        S_vv += nu_values[j] * nu_values[j];
        S_Av += A_values[j] * nu_values[j];
    }

    double r_hat = S_A / S_v;
    double delta = sqrt((double)cycle_count / (cycle_count - 1)) * 
                   sqrt(S_AA - 2 * r_hat * S_Av + r_hat * r_hat * S_vv) / S_v;

    double t_value = compute_t_value(cycle_count - 1);
    //double t_value = 2.920;
    return (t_value * delta <= 0.1 * r_hat);
}

int main() {
    srand(time(NULL));
    clock_t start_time = clock();

    // Initialize failures for each machine
    for (int i = 0; i < N; i++) {
        //schedule_event(ARRIVAL, generate_hyperexponential_random(), i);
        schedule_event(ARRIVAL, generate_exponential_random(LAMBDA_ARRIVAL), i);
    }
    regeneration_start_time = 0.0;

    while (!decide_to_stop()) {
        Event* current_event = pop_event();
        if (current_event == NULL) {
            break;
        }
        process_event(current_event);
        free(current_event);
    }

    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    double avg_waiting_time_long = total_waiting_time_long / cycle_count;

    printf("\n_______________________________________\n");
    printf("Simulation complete!\n");
    printf("Execution time: %f seconds\n", time_taken);
    printf("Total waiting time in long station: %f minutes\n", total_waiting_time_long);
    printf("Total regeneration cycles: %d\n", cycle_count);
    printf("Average waiting time for the long repair station: %f minutes\n", avg_waiting_time_long);
    //double avg_waiting_time_long_minutes = avg_waiting_time_long / 60.0;
    //printf("Average waiting time for the long repair station: %f minutes\n", avg_waiting_time_long_minutes);
    //std::cout << r_hat - delta << " CONFIDENCE_INTERVAL " << r_hat + delta << std::endl;
    return 0;
}
