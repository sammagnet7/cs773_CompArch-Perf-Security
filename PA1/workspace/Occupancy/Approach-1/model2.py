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

data = np.array(data)  # Shape: (5, 1000) assuming 5 categories, 1000 samples each

# Step 2: Flatten data (Each data point is now an independent sample)
features = []
labels = []

for category_label, category_data in enumerate(data):
    for value in category_data:
        features.append([value])  # Treat each value as an independent feature
        labels.append(category_label-1)  # Assign category label

# Convert to DataFrame
df = pd.DataFrame(features, columns=["Value"])
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
joblib.dump(clf, "single_point_classifier.pkl")
joblib.dump(scaler, "scaler.pkl")
print("Model and scaler saved!")

# Step 7: Evaluate the Model
y_pred = clf.predict(X_test)
accuracy = accuracy_score(y_test, y_pred)
print(f"Classifier Accuracy: {accuracy:.2f}")
print("\nClassification Report:\n", classification_report(y_test, y_pred))
