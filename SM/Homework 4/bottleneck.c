#include <stdio.h>

// Constants
#define M 4     // Number of servers (stations)

// Input parameters
double Z = 10.0;  // Think time for delay station (there is only one)
double S[M] = {0, 0.04, 0.06, 0.04};  // Service times
char ST[M] = {'D','C', 'C', 'C',};  // Station types ('C' for computational, 'D' for delay)
double V[M] = {1.0, 10, 5.5, 3.5};   // Visiting ratios (Vi)

int main() {
    double D[M];            // Service demands for each station
    double X_max;           // Maximum system throughput
    double X[M];            // Throughput
    double U[M];            // Utilization for each station
    double n[M];            // Mean queue length for each station
    double w[M];            // Mean waiting time for each station

    double maxD = 0.0;
    int bottleneck_station = 0;

    // Calculate service demand for each station and identify the bottleneck
    for (int i = 0; i < M; i++) {
        D[i] = V[i] * S[i];
        if (D[i] > maxD) {
            maxD = D[i];
            bottleneck_station = i;
        }
    }

    // Maximum throughput as N -> infinity: we can use this formula because we are in a closed system
    X_max = 1.0 / maxD;

    // Calculate utilization, mean queue length, and mean waiting time for each station
    for (int i = 0; i < M; i++) {
        //U[i] = X_max * D[i];  // Utilization
        X[i] = V[i]*X_max; //our reference station is the bottleneck station
        U[i] = X[i] * S[i];

        if (ST[i] == 'D') {
            // For the delay station
            n[i] = Z * X_max;
            w[i] = Z;
        } else {
            // For computational stations
            n[i] = U[i] / (1 - U[i]);    // Mean queue length using M/M/1 approximation
            w[i] = n[i] / X_max;         // Mean waiting time
        }
    }

    // Output bottleneck analysis results
    printf("Bottleneck Analysis as N -> infinity:\n\n");
    printf("The bottleneck station is %d, with the highest service demand that is %f\n", bottleneck_station, maxD);
    printf("Maximum Throughput (X_max) as N -> infinity: %f\n\n", X_max);

    // Print results for each station
    for (int i = 0; i < M; i++) {
        printf("Station %d:\n", i);
        printf("  Service Demand (D_%d(N))       = %f\n", i, D[i]);
        printf("  Throughput (X_%d(N))           = %f\n", i, X[i]);
        printf("  Utilization (U_%d(N))          = %f\n", i, U[i]);
        printf("  Mean Queue Length (n_%d(N))    = %f\n", i, n[i]);
        printf("  Mean Waiting Time (w_%d(N))    = %f\n\n", i, w[i]);
    }

    return 0;
}
