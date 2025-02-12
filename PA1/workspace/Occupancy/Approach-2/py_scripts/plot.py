import numpy as np
import matplotlib.pyplot as plt

# Read data from output.txt
file_path = "output.txt"

# Initialize list to store data for each category
data = []

with open(file_path, "r") as file:
    for line in file:
        numbers = list(map(float, line.split()))  # Convert space-separated numbers to float
        data.append(numbers)  # Store as a separate category

# Ensure all categories have 1000 samples
if not all(len(category) == 1000 for category in data):
    print("Warning: Not all categories have 1000 samples!")

# Define category labels
categories = [f"Category {i-1}" for i in range(len(data))]

# Create a box plot and extract statistics
plt.figure(figsize=(10, 6))
box_plot = plt.boxplot(data, labels=categories, patch_artist=True, showmeans=True)

# Extract and print statistics from the box plot
print("\nCategory Statistics (Extracted from Box Plot):\n")

for i, category in enumerate(categories):
    whisker_low = box_plot['whiskers'][2 * i].get_ydata()[1]  # Min whisker
    q1 = box_plot['caps'][2 * i].get_ydata()[0]  # Q1 (25th percentile)
    median = box_plot['medians'][i].get_ydata()[0]  # Median (50th percentile)
    q3 = box_plot['caps'][2 * i + 1].get_ydata()[0]  # Q3 (75th percentile)
    whisker_high = box_plot['whiskers'][2 * i + 1].get_ydata()[1]  # Max whisker
    outliers = box_plot["fliers"][i].get_ydata()  # Outliers

    print(f"{category}:")
    print(f"  Min (whisker): {whisker_low:.2f}")
    print(f"  Q1 (25th percentile): {q1:.2f}")
    print(f"  Median (50th percentile): {median:.2f}")
    print(f"  Q3 (75th percentile): {q3:.2f}")
    print(f"  Max (whisker): {whisker_high:.2f}")
    print(f"  Outliers: {len(outliers) if len(outliers) > 0 else 'None'}")
    print("-" * 40)

# Customization
plt.xlabel("Categories")
plt.ylabel("Values")
plt.title("Range Visualization of Categories from output.txt")
plt.grid(axis="y", linestyle="--", alpha=0.7)

# Show the plot
plt.savefig("filename.png", dpi=300)
