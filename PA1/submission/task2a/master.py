import subprocess
import signal
import os
import time
import sys

CHUNK_SIZE_BYTE = 7
STRT_BIT_SEQ = b"\xAA"  # 10101010
TERM_BIT_SEQ = b"\xAB"  # 10101011

SENDER_EXECUTABLE = "./sender"
RECEIVER_EXECUTABLE = "./receiver"
FILE_NAME_TO_SEND = ""
STRING_TO_SEND = ""
BIT_STRING_TO_SEND = ""
ENCODED_BIT_STRING_TO_SEND = ""
SENDER_INPUT_FILE = "processed.bin"
RECEIVER_OUTPUT_FILE = "received.txt"
DECODED_OUTPUT_FILE = "decoded.txt"

START_TIME = None
END_TIME = None
makefile_dir = "./"

def file_encoder(input_file: str = None, output_file: str = None, input_data: bytes = None):
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
    STRING_TO_SEND = input_data.decode('utf-8')
    output.extend(STRT_BIT_SEQ)
    
    for byte in input_data:
        if byte_number % CHUNK_SIZE_BYTE == 0 and byte_number != 0:
            output.extend(STRT_BIT_SEQ)
        output.append(byte)
        byte_number += 1
    
    output.extend(TERM_BIT_SEQ)
    
    if output_file:
        with open(output_file, "wb") as outfile:
            outfile.write(output)
    
    return output


def file_decoder(input_file: str, output_file: str):
    """
    Read input_file byte by byte, decode it, write to output_file, and return the decoded string.
    """
    byte_number = -1
    decoded_bytes = bytearray()
    
    with open(input_file, "rb") as infile, open(output_file, "wb") as outfile:
        while True:
            byte = infile.read(1)
            byte_number += 1
            if not byte:
                break
            if byte == STRT_BIT_SEQ:
                continue
            if byte == TERM_BIT_SEQ:
                break
            if byte_number % (CHUNK_SIZE_BYTE + 1) == 0:
                continue
            outfile.write(byte)
            decoded_bytes.append(byte[0])
    
    return decoded_bytes.decode('utf-8')

def calculate_accuracy(original_string: str, decoded_string: str) -> float:
    """
    Compare two strings and return their accuracy as a percentage.
    """
    matches = sum(1 for a, b in zip(original_string, decoded_string) if a == b)
    accuracy = (matches / max(len(original_string), len(decoded_string))) * 100
    return accuracy


############################## Build the executables ##############################
try:
    subprocess.run(["make", "clean"], cwd=makefile_dir, check=True)
    subprocess.run(["make"], cwd=makefile_dir, check=True)
    print("Build successful.")
except subprocess.CalledProcessError:
    print("Build failed.")
    sys.exit(1)


######################### Get input string to send #################################
FILE_NAME_TO_SEND = input("Enter file name: ")


################# Process input string ##################
ENCODED_BIT_STRING_TO_SEND = file_encoder(output_file=SENDER_INPUT_FILE, input_file=FILE_NAME_TO_SEND) 
#print(ENCODED_BIT_STRING_TO_SEND)
############# Start receiver #############
process_receiver = subprocess.Popen(["taskset", "-c", "2", RECEIVER_EXECUTABLE])
time.sleep(3)

try:
    ####### Start Sender with string as command-line argument #######
    START_TIME = time.time()   
    process_sender = subprocess.Popen(["taskset", "-c", "4", SENDER_EXECUTABLE])
 
    
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
        print(f"Sender exited with return code {process_sender.returncode}. Not terminating receiver.")

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
print(f"Original String: {STRING_TO_SEND}")
print(f"Decoded String: {decoded_string}")
print(f"Data Transmitted: {data_size_bits} bits")
print(f"Total Time Taken: {total_time:.6f} seconds")
print(f"Bandwidth: {bandwidth_bps:.2f} bps")
if len(decoded_string) == len(STRING_TO_SEND):
    accuracy = calculate_accuracy(STRING_TO_SEND, decoded_string)
    print(f"Communication Accuracy: {accuracy:.2f}%")

