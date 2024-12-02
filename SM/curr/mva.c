/* -------------------------------------------------------------------------
Università di Torino
M.S. in STOCHASTICS AND DATA SCIENCE
Course in Simulation
Homework 7

By Andrea Crusi and Lorenzo Sala
 * ------------------------------------------------------------------------- 
 */
#include <stdio.h>

// Constants
#define M 3    // Number of stations
#define N 10  // Maximum number of customers

// Input parameters
double Z = 3000;  // Delay time of the station
double S[M] = {0, 40, 0.95*10+0.05*19010};  // Service times
char ST[M] = {'D','L', 'L'};  // Station types ('L' for load independent, 'D' for delay. Our reference station is the delay)
double Q[M][M] = {
    {0, 1, 0},
    {0.8,0,0.2},
    {1,0,0}
};

double V[M];

// We need bi-dimensional arrays for results
double X[M][N+1];
double U[M][N+1]; 
double n[M][N+1];
double w[M][N+1];
double R[M][N+1];

int main() {
    // Initialization
    for (int i = 0; i < M; i++) {
        n[i][0] = 0.0;  // put 0 as the value for all stations
    }

    // Compute visit count
    V[0]=1.0;
    V[1]=V[0]/Q[0][1];
    V[2]=V[1]*Q[1][2];

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

        // Compute throughput for reference job. Remember that the reference station is the station 0 (delay station)
        double Xref = k / sum;

        // Compute performance metrics for each station i using MVA equations seen in class
        for (int i = 0; i < M; i++) {
            X[i][k] = V[i] * Xref;  // Throughput for station i

            if (ST[i] == 'D') {
                // Delay station
                n[i][k] = Z * X[i][k];
                U[i][k] = n[i][k] / k;
            } else {
                // Computational station
                U[i][k] = S[i] * X[i][k];  // Utilization
                n[i][k] = U[i][k] * (1 + n[i][k - 1]);  // Average queue length
            }
        }
        // Compute the response time
        double Y[M]={0.0,0.0,0.0};
        for (int i=0; i < M; i++) {
            
            for (int i = 0; i < M; i++) {
                Y[i] += V[i] * w[i][k];
            }
            if (ST[i] == 'D') {
                R[i][k] = (k/X[i][k])-Z; //formula for delay station
            } else {
                R[i][k] = w[i][N]+S[i];
            }
        }
    }
    int n_customers = N;
    // Print results for N = 1 and N = 80;
        printf("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
        printf("Simulation with %d customers\n\n", n_customers);
           for (int i = 0; i < M; i++) {
            printf("Station %d results:\n", i);
            printf("Throughput (X[%d])\t\t= %f\n", n_customers, X[i][n_customers]);
            printf("Utilization (U[%d])\t\t= %f\n", n_customers, U[i][n_customers]);
            printf("Mean queue length (n[%d])\t= %f\n", n_customers, n[i][n_customers]);
            printf("Mean waiting time (w[%d])\t= %f\n", n_customers, w[i][n_customers]);
            printf("Mean response time (R_0)\t\t= %f\n", (R[i][n_customers
        ])); 
            printf("\n");
    }
    
    double X_0_1=X[0][1]; // We will need this later to compute the saturation point

    return 0;
}
