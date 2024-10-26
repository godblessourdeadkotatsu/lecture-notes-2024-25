#include <stdio.h>

// Constants
#define M 4     // Number of servers (stations)
#define N 80  // Maximum number of customers

// Input parameters
double Z = 10.0;  // Think time for delay station (there is only one)
double S[M] = {0, 0.04, 0.06, 0.04};  // Service times
char ST[M] = {'D','L', 'L', 'L',};  // Station types ('L' for load independent, 'D' for delay)
//delay station is reference
// Visiting ratios (Vi)
double V[M] = {1.0, 10, 5.5, 3.5};

// Arrays for results
double X[M][N+1], U[M][N+1], n[M][N+1], w[M][N+1];

int main() {
    // Initialization
    for (int i = 0; i < M; i++) {
        n[i][0] = 0.0;  // put 0 as the value for all stations
    }

    // Compute performance measures for each population size k (from 1 to N)
    for (int k = 1; k <= N; k++) {

        // Compute the waiting time w_i[k] for each station i
        for (int i = 0; i < M; i++) {
            if (ST[i] == 'D') {
                w[i][k] = Z;  // Delay station
            } else {
                w[i][k] = S[i] * (1 + n[i][k - 1]);  // Queue station
            }
        }
        double sum = 0.0; // initialize sum

        // Compute the sum of Vi * wi[k] across all stations
        for (int i = 0; i < M; i++) {
            sum += V[i] * w[i][k];
        }

        // Compute throughput for reference job
        double Xref = k / sum;

        // Compute performance metrics for each station i
        for (int i = 0; i < M; i++) {
            X[i][k] = V[i] * Xref;  // Throughput for station i

            if (ST[i] == 'D') {
                // Delay station
                n[i][k] = Z * X[i][k];
                U[i][k] = n[i][k] / k;
            } else {
                // Computational station
                U[i][k] = S[i] * X[i][k];  // Utilization
                n[i][k] = U[i][k] * (1 + n[i][k - 1]);  // Number of customers
            }
        }
    }
    int n_customers[2] = {1,80};
    // Print results for N = 1 and N = 80;
    for (int j=0; j<=1; j++){
        printf("Simulation with %d customers\n\n", n_customers[j]);
           for (int i = 0; i < M; i++) {
            printf("Station %d results:\n", i);
            printf("Throughput (X[%d])\t\t= %f\n", n_customers[j], X[i][n_customers[j]]);
            printf("Utilization (U[%d])\t\t= %f\n", n_customers[j], U[i][n_customers[j]]);
            printf("Avg number of jobs (n[%d])\t= %f\n", n_customers[j], n[i][n_customers[j]]);
            printf("Avg waiting time (w[%d])\t= %f\n", n_customers[j], w[i][n_customers[j]]);
            printf("Avg response time (R_0)\t= %f\n", (n_customers[j]/X[0][n_customers[j]])-Z);
            printf("\n");
    }
        printf("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
    }

    getchar();
    return 0;
}
