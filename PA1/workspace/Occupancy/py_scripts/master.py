import subprocess
import signal
import os
import time
import sys
import warnings
warnings.simplefilter("ignore", category=UserWarning)

SENDER_EXECUTABLE = "../comm/sender"
RECEIVER_EXECUTABLE = "../comm/receiver"
STRING_TO_SEND = ""
BIT_STRING_TO_SEND = ""
ENCODED_BIT_STRING_TO_SEND = ""
RECEIVER_OUTPUT_FILE = "received.txt"
MODEL_PATH = "../models/dt_classifier_300_median_5.pkl"
SCALER_PATH = "../models/scaler.pkl"
START_TIME = None
END_TIME = None
makefile_dir = "../comm/"  # Change this to the actual path

############################## Build the executables ##############################
try:
    subprocess.run(["make", "clean"], cwd=makefile_dir, check=True)
    subprocess.run(["make"], cwd=makefile_dir, check=True)
    print("Build successful.")
except subprocess.CalledProcessError:
    print("Build failed.")
    sys.exit(1)


######################### Get input string to send #################################
STRING_TO_SEND = input("Input String: ")

def process_input(user_input):
    binary_str = ''.join(format(ord(char), '08b') for char in user_input)  # Convert to binary (8-bit)
    
    # Split into 2-bit chunks
    chunks = [binary_str[i:i+2] for i in range(0, len(binary_str), 2)]
    
    # Map 2-bit sequences to integers
    mapping = {"00": 0, "01": 1, "10": 2, "11": 3}
    processed_values = [mapping[ch] for ch in chunks]

    return (binary_str, ' '.join(map(str, processed_values)))  # Convert list to space-separated string


################# Process input string ##################
BIT_STRING_TO_SEND, ENCODED_BIT_STRING_TO_SEND = process_input(STRING_TO_SEND)
print(ENCODED_BIT_STRING_TO_SEND)
############# Start receiver #############
process_receiver = subprocess.Popen(["taskset", "-c", "2", RECEIVER_EXECUTABLE])
time.sleep(5)

try:
    ####### Start Sender with string as command-line argument #######
    START_TIME = time.time()
   # process_sender = subprocess.Popen(["taskset", "-c", "4", SENDER_EXECUTABLE, f'"{ENCODED_BIT_STRING_TO_SEND}"'])
    args = ENCODED_BIT_STRING_TO_SEND.split()     
    process_sender = subprocess.Popen(["taskset", "-c", "4", SENDER_EXECUTABLE] + args)
 
    
    ################ Wait for sender to exit #################
    process_sender.wait()
    END_TIME = time.time()

    ######## If sender exits successfully, terminate receiver ########
    if process_sender.returncode == 0:
        time.sleep(5)
        print("Sender exited successfully. Terminating Receiver...")
        process_receiver.terminate()  # Sends SIGTERM to receiver

        ###### Wait for receiver to handle the termination ######
        process_receiver.wait()
    else:
        print(f"Sender exited with return code {process_sender.returncode}. Not terminating receiver.")

except Exception as e:
    print(f"Error: {e}")
finally:
    ###### Ensure receiver is not left running ######
    if process_receiver.poll() is None:  # If receiver is still running
        process_receiver.terminate()
        process_receiver.wait()


############################ Inference  ##############################
import joblib
import numpy as np
import numpy as np
from sklearn.metrics import accuracy_score


################## Read data from receiver dump #######################
test_file = RECEIVER_OUTPUT_FILE 
data = []

with open(test_file, "r") as file:
    for line in file:
        numbers = list(map(float, line.split()))  # Convert space-separated numbers to float
        data.extend(numbers)  # Flatten into one list
## Filter 0s
data = [x for x in data if x != 0]
data = np.array(data).reshape(-1, 1)  # Convert to column vector

############################## Load Model and Predict ##################################

clf = joblib.load(MODEL_PATH)
scaler = joblib.load(SCALER_PATH)

data = scaler.transform(data)
predictions = clf.predict(data)
print("Original")
print(ENCODED_BIT_STRING_TO_SEND)
print("Detected")
print(predictions)
################################# Accuracy and Bandwidth ###############################
def decode_output(predicted_values):
    # Remove -1 values (inactive sender periods)
    filtered_values = [val for val in predicted_values if val != -1]
    
    # Mapping from integer to 2-bit binary
    reverse_mapping = {0: "00", 1: "01", 2: "10", 3: "11"}
    
    # Convert predicted values back to binary
    binary_str = ''.join(reverse_mapping[val] for val in filtered_values)
    
    # Split into 8-bit chunks and convert to characters
    decoded_chars = [chr(int(binary_str[i:i+8], 2)) for i in range(0, len(binary_str), 8)]
    
    return ''.join(decoded_chars)


# Decode predicted output
decoded_string = decode_output(predictions)
# Compute total transmission time
total_time = END_TIME - START_TIME

data_size_bits = len(BIT_STRING_TO_SEND)
# Calculate bandwidth
bandwidth_bps = data_size_bits / total_time  # in bits per second


################################### Print results #######################################
print(f"Original String: {STRING_TO_SEND}")
print(f"Decoded String: {decoded_string}")
print(f"Data Transmitted: {data_size_bits} bits")
print(f"Total Time Taken: {total_time:.6f} seconds")
print(f"Bandwidth: {bandwidth_bps:.2f} bps")
if len(decoded_string) == len(STRING_TO_SEND):
    accuracy = accuracy_score(list(STRING_TO_SEND), list(decoded_string))
    print(f"Communication Accuracy: {accuracy * 100:.2f}%")
