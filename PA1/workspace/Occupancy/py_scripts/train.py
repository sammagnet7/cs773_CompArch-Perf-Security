import numpy as np
import pandas as pd
import joblib
from sklearn.tree import DecisionTreeClassifier, plot_tree
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, classification_report

# Step 1: Read data from output.txt
file_path = "../calibration/output.txt"
data = []

with open(file_path, "r") as file:
    for line in file:
        numbers = list(map(float, line.split()))
        data.append(numbers)

data = np.array(data)

# Step 2: Preprocess the data
# Function to remove outliers row-wise using IQR
def remove_outliers_iqr(data):
    filtered_data = []
    
    for row in data:
        Q1 = np.percentile(row, 25)
        Q3 = np.percentile(row, 75)
        IQR = Q3 - Q1
        
        lower_bound = Q1 - 1.5 * IQR
        upper_bound = Q3 + 1.5 * IQR
        
        # Keep only values within bounds
        filtered_row = row[(row >= lower_bound) & (row <= upper_bound)]
        filtered_data.append(filtered_row)
    
    return filtered_data  # Returns a list of filtered rows with different lengths

filtered_data = remove_outliers_iqr(data)

# Step 3: Flatten data (Each data point is now independent)
features = []
labels = []

for category_label, category_data in enumerate(filtered_data):
    for value in category_data:
        features.append([value])
        labels.append(category_label-1)

# Convert to DataFrame
df = pd.DataFrame(features, columns=["Value"])
df["Category"] = labels

# Step 4: Train-Test Split
X = df.drop(columns=["Category"])
y = df["Category"]

# Step 5: Normalize Features
scaler = StandardScaler()
X = scaler.fit_transform(X)

# Step 6: Train a Decision Tree
clf_dt = DecisionTreeClassifier(
    max_depth=5,
    random_state=42
)

clf_dt.fit(X, y)

model_path = "../models/dt_classifier.pkl"
scaler_path = "../models/scaler.pkl"

# Step 7: Save the Model
joblib.dump(clf_dt, model_path)
joblib.dump(scaler, scaler_path)
print("Model and scaler saved!")