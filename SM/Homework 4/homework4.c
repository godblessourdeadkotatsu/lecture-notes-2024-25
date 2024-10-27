/* -------------------------------------------------------------------------
Università di Torion
M.S. in STOCHASTICS AND DATA SCIENCE
Course in Simulation
Homework 4

By Andrea Crusi and Lorenzo Sala
 * ------------------------------------------------------------------------- 
 */
#include <stdio.h>

// Constants
#define M 4     // Number of stations
#define N 80  // Maximum number of customers

// Input parameters
double Z = 10.0;  // Delay time of the station
double S[M] = {0, 0.04, 0.06, 0.04};  // Service times
char ST[M] = {'D','L', 'L', 'L',};  // Station types ('L' for load independent, 'D' for delay. Our reference station is the delay)
double Q[M][M] = {
    {0, 1, 0, 0},
    {0.1,0,0.55,0.35},
    {0,1,0,0},
    {0,1,0,0}
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
    V[1]=V[0]/Q[1][0];
    V[2]=V[1]*Q[1][2];
    V[3]=V[1]*Q[1][3];

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
        double Y[M]={0.0,0.0,0.0,0.0};
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
    int n_customers[2] = {1,N};
    // Print results for N = 1 and N = 80;
    for (int j=0; j<=1; j++){
        printf("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
        printf("Simulation with %d customers\n\n", n_customers[j]);
           for (int i = 0; i < M; i++) {
            printf("Station %d results:\n", i);
            printf("Throughput (X[%d])\t\t= %f\n", n_customers[j], X[i][n_customers[j]]);
            printf("Utilization (U[%d])\t\t= %f\n", n_customers[j], U[i][n_customers[j]]);
            printf("Mean queue length (n[%d])\t= %f\n", n_customers[j], n[i][n_customers[j]]);
            printf("Mean waiting time (w[%d])\t= %f\n", n_customers[j], w[i][n_customers[j]]);
            //printf("Mean response time (R_0)\t\t= %f\n", (n_customers[j]/X[i][n_customers[j]])-Z);
            printf("Mean response time (R_0)\t\t= %f\n", (R[i][n_customers
            [j]])); 
            printf("\n");
    }
    }
    
    double X_0_1=X[0][1]; // We will need this later to compute the saturation point

    // We now make a pair of csv files that we will plot with pgfplots in the homework discussion
    FILE *file = fopen("x0.csv", "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return 1;
    }

        for (int k = 1; k <= N; k++) {
            fprintf(file, "%d %f\n", k, X[0][k]);
        }

    fclose(file);
    printf("Data written to x0.csv successfully.\n");

    // We now save the results of the response time for station 0
    FILE *file2 = fopen("R0.csv", "w");
    if (file2 == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return 1;
    }

        for (int k = 1; k <= N; k++) {
            fprintf(file2, "%d %f\n", k, R[0][k]);
        }

    fclose(file2);
    printf("Data written to R0.csv successfully.\n");
    
    // We now save the results of average queue length
    FILE *file3 = fopen("queue.csv", "w");
    if (file3 == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return 1;
    }

    fprintf(file3, "n station0 station1 station2 station3\n");

        for (int k = 1; k <= N; k++) {
            fprintf(file3, "%d %f %f %f %f\n", k, n[0][k],n[1][k],n[2][k],n[3][k]);
        }

    fclose(file3);
    printf("Data written to queue.csv successfully.\n");

       // We now save the results of utilization
    FILE *file4 = fopen("utilization.csv", "w");
    if (file4 == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return 1;
    }

    fprintf(file4, "n station0 station1 station2 station3\n");

        for (int k = 1; k <= N; k++) {
            fprintf(file4, "%d %f %f %f %f\n", k, U[0][k],U[1][k],U[2][k],U[3][k]);
        }

    fclose(file4);
    printf("Data written to utilization.csv successfully.\n");

    // Now we perform the bottleneck analysis using the equations derived in the previous exercise.
    printf("\n");
    
    double D[M];            // Service demands for each station
    double Xb;           // Variable to store the max throughput of the bottleneck station
    double X[M];            // Throughput
    double U[M];            // Utilization for each station
    double n[M];            // Mean queue length for each station
    double w[M];            // Mean waiting time for each station

    double Db = 0.0;
    int bottleneck_station = 0;

    // Calculate service demand for each station and identify the bottleneck as the station with higher service demand
    for (int i = 0; i < M; i++) {
        D[i] = V[i] * S[i];
        if (D[i] > Db) {
            Db = D[i];
            bottleneck_station = i;
        }
    }

    // Maximum throughput as N -> infinity: we can use this formula because we are in a closed system
    Xb = 1.0 / Db;

    // Calculate utilization, mean queue length, and mean waiting time for each station
    for (int i = 0; i < M; i++) {
        X[i] = V[i]*Xb; //our reference station is the bottleneck station
        U[i] = X[i] * S[i];

        if (ST[i] == 'D') {
            // For the delay station
            n[i] = Z * Xb;
            w[i] = Z;
        } else {
            // For load independent station stations
            n[i] = U[i] / (1 - U[i]);    // Mean queue length 
            w[i] = n[i] / Xb;         // Mean waiting time
        }
    }

    // Print the bottleneck analysis results
    printf("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n");
    printf("Bottleneck Analysis (using formulas of the previous exercise)\n");
    printf("=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+\n\n");
    printf("The bottleneck station is %d, with the highest service demand that is %f\n", bottleneck_station, Db);
    printf("Maximum Throughput (Xb) as N -> infinity: %f\n\n", Xb);

    // Print results for each station
    for (int i = 0; i < M; i++) {
        printf("Station %d:\n", i);
        printf("  Service Demand (D_%d(N))       = %f\n", i, D[i]);
        printf("  Throughput (X_%d(N))           = %f\n", i, X[i]);
        printf("  Utilization (U_%d(N))          = %f\n", i, U[i]);
        printf("  Mean Queue Length (n_%d(N))    = %f\n", i, n[i]);
        printf("  Mean Waiting Time (w_%d(N))    = %f\n\n", i, w[i]);
    }
    printf("_._._._._._._._._._._._._._._._._._._._._._._._._._._._._._.\n");
    printf("For plotting purposes:\n");
    printf("Vb*Sb=Db=%f\n",Db);
    printf("Point of saturation N_star: %f clients",(1/(X_0_1*Db)));
    return 0;
}
