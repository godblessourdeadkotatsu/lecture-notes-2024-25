#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Constants
#define Z_ALPHA_2 1.96 // 95% confidence interval
#define BETA 0.2       // Probability of long repair
#define MEAN_FAILURE_TIME 3000.0
#define MEAN_SHORT_REPAIR 40.0
#define MEAN_LONG_REPAIR 960.0
#define MIN_CYCLES 40

// Function prototypes
double exponential_random(double mean);
void simulate_cycle(double *long_wait_total, int *cycle_count);
void compute_confidence_interval(int n, double mean, double variance, double *lower, double *upper);

int main() {
    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Variables for simulation
    double long_wait_total = 0.0;
    int cycle_count = 0;
    double long_wait_mean, long_wait_variance;
    double confidence_interval_lower, confidence_interval_upper;
    double half_width;

    do {
        // Simulate one regeneration cycle
        simulate_cycle(&long_wait_total, &cycle_count);

        // Calculate statistics
        long_wait_mean = long_wait_total / cycle_count;

        // Variance approximation (requires at least 2 cycles)
        if (cycle_count > 1) {
            double temp_sum = 0.0;
            for (int i = 0; i < cycle_count; i++) {
                temp_sum += pow(long_wait_mean - (long_wait_total / cycle_count), 2);
            }
            long_wait_variance = temp_sum / (cycle_count - 1);
        } else {
            long_wait_variance = 0.0; // Insufficient cycles for variance
        }

        // Compute confidence interval
        compute_confidence_interval(cycle_count, long_wait_mean, long_wait_variance, &confidence_interval_lower, &confidence_interval_upper);

        // Calculate half-width of confidence interval
        half_width = (confidence_interval_upper - confidence_interval_lower) / 2;

    } while (half_width / long_wait_mean > 0.05 || cycle_count < MIN_CYCLES);

    // Print results
    printf("Simulation completed.\n");
    printf("Average waiting time at long repair station: %.2f minutes\n", long_wait_mean);
    printf("95%% Confidence Interval: [%.2f, %.2f]\n", confidence_interval_lower, confidence_interval_upper);
    printf("Total regeneration cycles: %d\n", cycle_count);

    return 0;
}

// Generate random numbers with exponential distribution
double exponential_random(double mean) {
    double u = (double)rand() / RAND_MAX; // Uniform random number in [0,1]
    return -mean * log(1 - u);
}

// Simulate one regeneration cycle
void simulate_cycle(double *long_wait_total, int *cycle_count) {
    double system_time = 0.0; // Current simulation time
    double next_failure = exponential_random(MEAN_FAILURE_TIME);
    double short_repair_time, long_repair_time;
    double long_wait = 0.0;

    // Simulate until the system becomes empty (regeneration point)
    while (next_failure <= system_time) {
        // Determine if the repair is short or long
        double is_long_repair = ((double)rand() / RAND_MAX) < BETA;

        if (is_long_repair) {
            long_repair_time = exponential_random(MEAN_LONG_REPAIR);
            long_wait += long_repair_time;
        } else {
            short_repair_time = exponential_random(MEAN_SHORT_REPAIR);
        }

        // Advance the system time
        system_time = next_failure;
        next_failure = system_time + exponential_random(MEAN_FAILURE_TIME);
    }

    // Add to totals and increase cycle count
    *long_wait_total += long_wait;
    (*cycle_count)++;
}

// Compute confidence interval
void compute_confidence_interval(int n, double mean, double variance, double *lower, double *upper) {
    double margin_of_error = Z_ALPHA_2 * sqrt(variance / n);
    *lower = mean - margin_of_error;
    *upper = mean + margin_of_error;
}
