import joblib
import numpy as np

# Load the trained model and scaler
model_filename = "single_point_classifier.pkl"
scaler_filename = "scaler.pkl"

clf = joblib.load(model_filename)
scaler = joblib.load(scaler_filename)
print("Model and scaler loaded!")

# Read test data from file
test_file = "input.txt"  # Your test data file
data = []

with open(test_file, "r") as file:
    for line in file:
        numbers = list(map(float, line.split()))  # Convert space-separated numbers to float
        data.extend(numbers)  # Flatten into one list

data = np.array(data).reshape(-1, 1)  # Convert to column vector
print(f"Loaded {len(data)} test samples.")

# Normalize the data
data = scaler.transform(data)

# Predict categories
predictions = clf.predict(data)

# Save predictions to a file
output_file = "predictions.txt"
with open(output_file, "w") as f:
    f.write(" ".join(map(str, predictions)))  # Save space-separated predictions

print(f"Predictions saved to {output_file} ✅")
