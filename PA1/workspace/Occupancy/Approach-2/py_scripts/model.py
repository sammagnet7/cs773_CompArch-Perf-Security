import numpy as np
import pandas as pd
import joblib
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, classification_report

# Step 1: Read data from output.txt
file_path = "output.txt"
data = []

with open(file_path, "r") as file:
    for line in file:
        numbers = list(map(float, line.split()))  # Convert space-separated numbers to float
        data.append(numbers)  # Store each line as a category

data = np.array(data)  # Shape: (5, 1000)

# Step 2: Create Windowed Data (Sliding Window of 4 Samples)
features = []
labels = []
window_size = 4

for category_label, category_data in enumerate(data):
    for i in range(len(category_data) - window_size + 1):  # Sliding window
        window = category_data[i : i + window_size]  # Take 4 consecutive samples
        features.append([
            np.mean(window),  # Mean
            np.median(window),  # Median
            np.var(window),  # Variance
            np.min(window),  # Min
            np.max(window)   # Max
        ])
        labels.append(category_label-1)  # Assign category label

# Convert to DataFrame
df = pd.DataFrame(features, columns=["Mean", "Median", "Variance", "Min", "Max"])
df["Category"] = labels

# Step 3: Train-Test Split
X = df.drop(columns=["Category"])
y = df["Category"]
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42, stratify=y)

# Step 4: Normalize Features
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

# Step 5: Train the Model
clf = RandomForestClassifier(n_estimators=100, random_state=42)
clf.fit(X_train, y_train)

# Step 6: Save the Model
joblib.dump(clf, "windowed_category_classifier.pkl")
joblib.dump(scaler, "scaler.pkl")
print("Model and scaler saved!")

# Step 7: Evaluate the Model
y_pred = clf.predict(X_test)
accuracy = accuracy_score(y_test, y_pred)
print(f"Classifier Accuracy: {accuracy:.2f}")
print("\nClassification Report:\n", classification_report(y_test, y_pred))
