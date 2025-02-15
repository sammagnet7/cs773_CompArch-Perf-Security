import subprocess
import signal
import os
import sys
import time
import sys

SENDER_EXECUTABLE = "bin/sender"
RECEIVER_EXECUTABLE = "bin/receiver"
FILE_NAME_TO_SEND = ""
STRING_TO_SEND = ""
BIT_STRING_TO_SEND = ""
ENCODED_BIT_STRING_TO_SEND = ""
SENDER_INPUT_FILE = "processed.bin"
RECEIVER_OUTPUT_FILE = "received.txt"
DECODED_OUTPUT_FILE = ""

STRT_BIT_SEQ = b'\xaa'
TERM_BIT_SEQ = b'\xdb'
CHUNK_SIZE_BYTE = 32
START_TIME = None
END_TIME = None
makefile_dir = "./"

import reedsolo
 
# Reed-Solomon encoder/decoder (tweak `nsym` to change error correction strength)
RS = reedsolo.RSCodec(nsym=16)  # 16 parity bytes for error correction
 
def file_encoder(input_file: str = None, output_file: str = None, input_data: bytes = None):
    """
    Encode input data with error correction and write to output_file.
    """
    if input_data is None and input_file is None:
        raise ValueError("Either input_file or input_data must be provided.")
 
    byte_number = 0
    output = bytearray()
 
    if input_file:
        with open(input_file, "rb") as infile:
            input_data = infile.read()
 
    global STRING_TO_SEND
    STRING_TO_SEND = input_data
 
    output.extend(STRT_BIT_SEQ)
 
    for i in range(0, len(input_data), CHUNK_SIZE_BYTE):
        chunk = input_data[i:i + CHUNK_SIZE_BYTE]
        encoded_chunk = RS.encode(chunk)  # Apply error correction
        output.extend(encoded_chunk)
        byte_number += len(chunk)
 
    output.extend(TERM_BIT_SEQ)
 
    if output_file:
        with open(output_file, "wb") as outfile:
            outfile.write(output)
 
    return output
 
 
def file_decoder(input_file: str, output_file: str):
    """
    Read input_file, decode with error correction, write to output_file.
    """
    decoded_bytes = bytearray()
 
    with open(input_file, "rb") as infile, open(output_file, "wb") as outfile:
        data = infile.read()
 
        #if not data.startswith(STRT_BIT_SEQ) or not data.endswith(TERM_BIT_SEQ):
         #   raise ValueError("Invalid encoded file format")
 
        # Remove start/termination markers
        data = data[len(STRT_BIT_SEQ):-len(TERM_BIT_SEQ)]
 
        for i in range(0, len(data), CHUNK_SIZE_BYTE + 16):  # 16 bytes for error correction
            chunk = data[i:i + CHUNK_SIZE_BYTE + 16]
            try:
                decoded_chunk = RS.decode(chunk)  # Error correction
                decoded_bytes.extend(decoded_chunk)
                outfile.write(decoded_chunk)
            except reedsolo.ReedSolomonError:
                print(f"Warning: Failed to correct errors in chunk {i // CHUNK_SIZE_BYTE}")
 
    return decoded_bytes
file_decoder("received.txt", "out.txt")
def file_encoder(
    input_file: str = None, output_file: str = None, input_data: bytes = None
):
    """
    Encode input data and write to output_file. Can take either an input file or a direct byte string.
    """
    if input_data is None and input_file is None:
        raise ValueError("Either input_file or input_data must be provided.")

    byte_number = 0
    output = bytearray()

    if input_file:
        with open(input_file, "rb") as infile:
            input_data = infile.read()
    global STRING_TO_SEND
    STRING_TO_SEND = input_data

    for byte in input_data:
        output.append(byte)
        byte_number += 1

    if output_file:
        with open(output_file, "wb") as outfile:
            outfile.write(output)

    return output



def calculate_accuracy(original_file: str, decoded_file: str) -> float:
    """
    Open and compare two files and return their accuracy as a percentage.
    Note: Files may be non-text.
    """
    with open(original_file, "rb") as f1, open(decoded_file, "rb") as f2:
        original_bytes = f1.read()
        decoded_bytes = f2.read()

    if not original_bytes:
        return 100.0 if not decoded_bytes else 0.0

    matching_bytes = sum(b1 == b2 for b1, b2 in zip(original_bytes, decoded_bytes))
    total_bytes = max(len(original_bytes), len(decoded_bytes))

    return (matching_bytes / total_bytes) * 100 if total_bytes > 0 else 100.0


############################## Build the executables ##############################
try:
    subprocess.run(["make", "clean"], cwd=makefile_dir, check=True)
    subprocess.run(["make"], cwd=makefile_dir, check=True)
    print("Build successful.")
except subprocess.CalledProcessError:
    print("Build failed.")
    sys.exit(1)


######################### Get input string to send #################################
FILE_NAME_TO_SEND="msg.txt"
if len(sys.argv) > 1:
    FILE_NAME_TO_SEND = sys.argv[1]
else:
    FILE_NAME_TO_SEND = input("Enter file name: ")

if len(sys.argv) > 2:
    DECODED_OUTPUT_FILE = sys.argv[2]


################# Process input string ##################
ENCODED_BIT_STRING_TO_SEND = file_encoder(
    output_file=SENDER_INPUT_FILE, input_file=FILE_NAME_TO_SEND
)
# print(ENCODED_BIT_STRING_TO_SEND)
############# Start receiver #############
process_receiver = subprocess.Popen(["taskset", "-c", "2", RECEIVER_EXECUTABLE, FILE_NAME_TO_SEND])
time.sleep(3)

try:
    ####### Start Sender with string as command-line argument #######
    START_TIME = time.time()
    process_sender = subprocess.Popen(["taskset", "-c", "4", SENDER_EXECUTABLE, FILE_NAME_TO_SEND])

    ################ Wait for sender to exit #################
    process_sender.wait()
    END_TIME = time.time()

    ######## If sender exits successfully, terminate receiver ########
    if process_sender.returncode == 0:
        time.sleep(3)
        print("Sender exited successfully. Terminating Receiver...")
        process_receiver.send_signal(signal.SIGINT)  # Sends SIGINT to receiver

        ###### Wait for receiver to handle the termination ######
        process_receiver.wait()
    else:
        print(
            f"Sender exited with return code {process_sender.returncode}. Not terminating receiver."
        )

except Exception as e:
    print(f"Error: {e}")
finally:
    ###### Ensure receiver is not left running ######
    if process_receiver.poll() is None:  # If receiver is still running
        process_receiver.terminate()
        process_receiver.wait()

################################# Accuracy and Bandwidth ###############################

# Decode output
decoded_string = file_decoder(RECEIVER_OUTPUT_FILE, DECODED_OUTPUT_FILE)
# Compute total transmission time
total_time = END_TIME - START_TIME

data_size_bits = len(ENCODED_BIT_STRING_TO_SEND)
# Calculate bandwidth
bandwidth_bps = data_size_bits / total_time  # in bits per second
print(len(decoded_string), len(STRING_TO_SEND))

################################### Print results #######################################
print(f"Data Transmitted: {data_size_bits} bits")
print(f"Total Time Taken: {total_time:.6f} seconds")
print(f"Bandwidth: {bandwidth_bps:.2f} bps")
# if len(decoded_string) == len(ENCODED_BIT_STRING_TO_SEND):
accuracy = calculate_accuracy(FILE_NAME_TO_SEND, DECODED_OUTPUT_FILE)
print(f"Communication Accuracy: {accuracy:.2f}%")
