import matplotlib.pyplot as plt

# File name containing the data
file_name = "log2mod.txt"

# Variables to store the data
lower_bounds = []
upper_bounds = []
simulation_numbers = []

# Read the data from the file
with open(file_name, "r") as file:
    for i, line in enumerate(file):
        lower, upper = map(float, line.strip().split(","))
        lower_bounds.append(lower)
        upper_bounds.append(upper)
        simulation_numbers.append(i + 1)  # Simulation number (1-based index)

# Plotting the intervals
plt.figure(figsize=(10, 8))

# Draw a horizontal line for each interval
for sim_num, lower, upper in zip(simulation_numbers, lower_bounds, upper_bounds):
    plt.plot([lower, upper], [sim_num, sim_num], 'k-', linewidth=0.7)  # Black line

# Add a vertical line for the target mean value (adjust this as needed)
mean_value = 1811.030708  # Replace with your target value if applicable
plt.axvline(x=mean_value, color='b', linestyle='dashdot', label='Mean Value')

# Customize the plot
plt.title("Confidence Intervals Comparison", fontsize=14)
plt.xlabel("Confidence Interval", fontsize=12)
plt.ylabel("Simulation Number", fontsize=12)
plt.grid(True, linestyle='--', alpha=0.5)
plt.gca().set_facecolor("#f0f0f5")
plt.tight_layout()
plt.legend()

# Save the plot to a file
plt.savefig("confidence_intervals2_mod.png", dpi=300)
print("Plot saved as 'confidence_intervals2_mod.png'.")

# Show the plot
plt.show()