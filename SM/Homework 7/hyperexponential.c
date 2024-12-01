#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

double alpha[] = {0.95,0.05};
double mu[] = {10, 19010};

double uniform_random() {
    return (double)rand() / RAND_MAX;  // Generates a uniform random number in [0,1]
}

double exponential_random(double lambda) {
    double u = (double)rand() / RAND_MAX;
    if ((1 - u) < 1e-20) {
    u = 1e-20; // this will give us a limit in the case in which the argument
                  // of the logarithm is too close to 0 (and thus exploding)
  }
    return -log(1 - u) * lambda;  // Inverse transform method
}

double hyperexponential_random(int k, double* p, double* lambda) {
    double u = (double)rand() / RAND_MAX;
    double cumulative_probability = 0.0;

    // Choose the component
    for (int i = 0; i < k; i++) {
        cumulative_probability += p[i];
        if (u < cumulative_probability) {
            return exponential_random(lambda[i]);  // Sample from chosen exponential
        }
    }

    // Fallback (should not reach here if probabilities sum to 1)
    return exponential_random(lambda[k-1]);
}

int main() {
    srand(time(NULL));    
    // Example: Hyperexponential with 2 components
    int k = 2;
    double p[] = {0.95, 0.05};     // Probabilities
    double lambda[] = {10, 19010}; // Rates

    double somma = 0;
    int max = 10000;
    // Generate 10 samples
    for (int i = 0; i < max; i++) {
        double sample = hyperexponential_random(k, p, lambda);
        somma += sample;
        printf("Sample %d: %f\n", i + 1, sample);
    }
    printf("media=%f",somma/max);
    return 0;
}
