import joblib
import numpy as np

# Load the trained model and scaler
model_filename = "windowed_category_classifier.pkl"
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

data = np.array(data)  # Convert to NumPy array
print(f"Loaded {len(data)} test samples.")

# Process in windows of 4
window_size = 4
predictions = []

for i in range(len(data) - window_size + 1):  # Sliding window
    window = data[i : i + window_size]  # Take 4 consecutive samples
    features = np.array([
        np.mean(window),
        np.median(window),
        np.var(window),
        np.min(window),
        np.max(window)
    ]).reshape(1, -1)  # Reshape for model input

    # Normalize the features
    features = scaler.transform(features)

    # Predict the category
    predicted_category = clf.predict(features)[0]
    predictions.append(predicted_category)

# Save predictions to a file
output_file = "predictions.txt"
with open(output_file, "w") as f:
    f.write(" ".join(map(str, predictions)))  # Save space-separated predictions

print(f"Predictions saved to {output_file} ✅")
