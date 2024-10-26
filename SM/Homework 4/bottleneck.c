#include <stdio.h>

// Constants
#define M 4     // Number of servers (stations)

// Input parameters
double Z = 10.0;  // Think time for delay station (there is only one)
double S[M] = {0, 0.04, 0.06, 0.04};  // Service times
char ST[M] = {'D','C', 'C', 'C',};  // Station types ('C' for computational, 'D' for delay)
double V[M] = {1.0, 10, 5.5, 3.5};   // Visiting ratios (Vi)

int main() {
    double D[M];   // Service demands for each station
    double maxD = 0.0;
    int bottleneck_station = 0;

    // Calculate the service demand for each station and identify the bottleneck
    for (int i = 0; i < M; i++) {
        D[i] = V[i] * S[i];
        if (D[i] > maxD) {
            maxD = D[i];
            bottleneck_station = i;
        }
    }

    // Calculate the maximum throughput as N -> infinity
    double X_max = 1.0 / maxD;

    // Output bottleneck analysis results
    printf("Bottleneck Analysis:\n");
    for (int i = 0; i < M; i++) {
        printf("Station %d - Service Demand (D[%d]) = %f\n", i, i, D[i]);
    }
    printf("\nBottleneck Station: %d with Service Demand = %f\n", bottleneck_station, maxD);
    printf("Maximum Throughput (X_max) as N -> infinity: %f\n", X_max);

    return 0;
}