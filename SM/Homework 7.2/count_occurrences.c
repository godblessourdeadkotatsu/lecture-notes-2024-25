#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("iter.txt", "r");
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