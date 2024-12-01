#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double exponential_random(double heta) {
    double unif = (double)rand() / RAND_MAX;
    if ((1 - unif) < 1e-20) {
        unif = 1e-20; // Prevent log(0) issues
    }
    double exp = (-heta * logf(1 - unif));
    return exp;
}

double hyperexponential_random(int k, double* alpha, double* eta) {
    // Cumulative distribution for alpha
    double cumulative_alpha[k];
    cumulative_alpha[0] = alpha[0];
    for (int i = 1; i < k; i++) {
        cumulative_alpha[i] = cumulative_alpha[i - 1] + alpha[i];
    }

    // Generate a uniform random number
    double Y = (double)rand() / RAND_MAX;

    // Select the component distribution
    int j = 0;
    while (Y > cumulative_alpha[j]) {
        j++;
    }

    // Generate a random variable from the chosen exponential distribution
    double X = exponential_random(eta[j]);

    return X;
}

double expected_value(int k, double* alpha, double* eta) {
    double E = 0.0;
    for (int i = 0; i < k; i++) {
        E += alpha[i] * eta[i];
    }
    return E;
}

int main() {
    // Example parameters for the hyperexponential distribution
    int k = 2; // Number of components
    double alpha[] = {0.95, 0.05}; // Weights (must sum to 1)
    double eta[] = {10, 19010};   // Mean values of the exponential distributions

    // Seed random number generator
    srand((unsigned)time(NULL));

    // Number of samples to generate
    int num_samples = 100000;
    double sum = 0.0;

    // Generate samples and calculate the sum
    for (int i = 0; i < num_samples; i++) {
        sum += hyperexponential_random(k, alpha, eta);
    }

    // Calculate and print the average
    double average = sum / num_samples;
    printf("Average of generated samples: %f\n", average);

    // Calculate and print the expected value
    double E = expected_value(k, alpha, eta);
    printf("Expected value: %f\n", E);

    // Compare the two
    printf("Difference (Average - Expected): %f\n", average - E);

    return 0;
}
