import matplotlib.pyplot as plt
import pandas as pd
from io import StringIO

# Your data
data = ""
with open('data/data.csv', 'r') as fileobj:
    data = fileobj.read()

# Read the data into a pandas DataFrame
df = pd.read_csv(StringIO(data))

# Extract mode names (removing 'mirage_' prefix)
df['mode'] = df['mode'].str.replace('mirage_', '')

# Get unique benchmarks
benchmarks = df['benchmark'].unique()

# Create a figure
plt.figure(figsize=(12, 6))

# Use a colormap with enough colors
colors = plt.cm.tab20.colors  # Using tab20 which has 20 distinct colors

# Plot a line for each benchmark
for i, benchmark in enumerate(benchmarks):
    benchmark_data = df[df['benchmark'] == benchmark]
    plt.plot(benchmark_data['mode'], benchmark_data['l2_replacement'], 
             marker='o', color=colors[i], label=benchmark)

# Set labels and title
plt.xlabel('L2 Tag Overhead (%) of baseline tag size', fontsize=16)
plt.ylabel('Local Replacement', fontsize=16)
plt.title('Mirage Tradeoff :: Tag Overhead vs Security', fontsize=16, pad=20)

# Rotate x-axis labels for better readability
plt.xticks(rotation=45, fontsize=14)
plt.yticks(fontsize=14)

# Use logarithmic scale for y-axis due to large value range
# plt.yscale('log')

# Add legend
plt.legend(title='Benchmark', title_fontsize=14, fontsize=14)
# Adjust layout to prevent label cutoff
plt.tight_layout()

# Show the plot
plt.savefig('plots/replacements.png', dpi=300, bbox_inches='tight')