import matplotlib.pyplot as plt
import pandas as pd
from io import StringIO
from matplotlib.ticker import MaxNLocator

# Your data
data = ""
with open('data/data.csv', 'r') as fileobj:
    data = fileobj.read()

# Read the data into a pandas DataFrame
df = pd.read_csv(StringIO(data))

# Extract mode names (removing 'mirage_' prefix)
df['mode'] = df['mode'].str.replace('ghost_ghost_', '')

# Get unique benchmarks
benchmarks = df['benchmark'].unique()

# Create a figure
plt.figure(figsize=(12, 6))

# Use a colormap with enough colors
colors = plt.cm.tab20.colors  # Using tab20 which has 20 distinct colors

# Plot a line for each benchmark
for i, benchmark in enumerate(benchmarks):
    benchmark_data = df[df['benchmark'] == benchmark]
    plt.plot(benchmark_data['mode'], benchmark_data['simSeconds'], 
             marker='o', color=colors[i], label=benchmark)

# plt.gca().yaxis.set_major_locator(MaxNLocator(nbins=6))
# Set labels and title
plt.xlabel('GhostMinion Size', fontsize=16)
plt.ylabel('Execution time', fontsize=16)
plt.title('GhostMinion Tradeoff :: GhostMinion Size vs Execution Time', fontsize=16, pad=20)

# Rotate x-axis labels for better readability
plt.xticks(rotation=45, fontsize=14)
plt.yticks(fontsize=14)

# Use logarithmic scale for y-axis due to large value range
# plt.yscale('log')

# Add legend
plt.legend(title='Benchmark', title_fontsize=14, fontsize=14, bbox_to_anchor=(1, 1), loc='upper left')
# Adjust layout to prevent label cutoff
plt.tight_layout()

# Show the plot
plt.savefig('plots/speedups.png', dpi=300, bbox_inches='tight',)