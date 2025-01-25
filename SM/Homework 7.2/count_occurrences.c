#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Check if the file name is provided as an argument
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    double lower, upper;
    double target = 1811.030708;
    int count = 0;

    // Read each line of the file
    while (fscanf(file, "%lf,%lf", &lower, &upper) == 2) {
        // Check if the target is within the interval
        if (target >= lower && target <= upper) {
            count++;
        }
    }

    fclose(file);

    // Print the result
    printf("The number %f falls inside %d intervals.\n", target, count);

    return 0;
}