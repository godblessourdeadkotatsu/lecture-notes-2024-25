#include <stdio.h>

// Constants
#define M 10  // Number of stations (can be adjusted)
#define N 100 // Max number of customers (can be adjusted)

// Input variables
double V[M];       // Visit ratios
double S[M][N];    // Service times
double X[M][N];    // Throughputs
double U[M][N];    // Utilizations
double R[M][N];    // Response times
double w[M];       // Waiting times
double p[M][N];    // Probability of k customers at station i
double f[M];       // Service factors

// Output variables
double t[M];       // Time spent at station i
double D[M];       // Queue length at station i
double CUM, WAIT, Y, Xref; // Cumulative variables

// Function to initialize the algorithm
void initialize() {
    CUM = 0.0;
    WAIT = 0.0;
    Y = 0.0;

    for (int i = 0; i < M; i++) {
        p[i][0] = 1.0;
        for (int j = 1; j <= N; j++) {
            p[i][j] = 0.0;
        }
    }
}

// Function to compute performance measures
void compute() {
    for (int k = 1; k <= N; k++) {
        WAIT = 0.0;

        // Compute WAIT and w_i
        for (int i = 0; i < M; i++) {
            for (int j = 1; j <= k; j++) {
                WAIT += j * S[i][j] * p[i][j - 1];
            }
            w[i] = WAIT;
        }

        // Compute Y
        for (int i = 0; i < M; i++) {
            Y += V[i] * w[i];
        }

        Xref = k / Y;

        // Compute X_i, U_i, and cumulative probability
        for (int i = 0; i < M; i++) {
            X[i][k] = Xref * V[i];
            U[i][k] = X[i][k] * S[i][k];
            CUM = 0.0;
            
            for (int j = 1; j <= k; j++) {
                p[i][j] = (X[i][k] * S[i][j] * p[i][j - 1] * (k - 1)) / k;
                CUM += p[i][j];
            }

            // Adjust cumulative probability
            p[i][k] = 1.0 - CUM;

            // Compute performance metrics
            D[i] = CUM;
            t[i] = X[i][k] * w[i];
        }
    }
}

int main() {
    // Initialize and compute performance figures
    initialize();
    compute();

    // Display results (for debugging, you can print specific values here)
    for (int i = 0; i < M; i++) {
        printf("Station %d: Throughput = %f, Utilization = %f\n", i, X[i][N], U[i][N]);
    }

    return 0;
}
