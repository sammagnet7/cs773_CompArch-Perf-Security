import sys

CHUNK_SIZE_BYTE = 7
STRT_BIT_SEQ = b"\xAA"  # 10101010
TERM_BIT_SEQ = b"\xAB"  # 10101011


def file_encoder(input_file: str, output_file: str):
    """
    Read input_file byte by byte, encode it, and write to output_file.
    """
    byte_number = 0
    with open(input_file, "rb") as infile, open(output_file, "wb") as outfile:
        while True:
            if (byte_number) % CHUNK_SIZE_BYTE == 0:
                outfile.write(STRT_BIT_SEQ)
            byte = infile.read(1)
            byte_number += 1
            if not byte:
                break
            outfile.write(byte)
        outfile.write(TERM_BIT_SEQ)


def file_decoder(input_file: str, output_file: str):
    """
    Read input_file byte by byte, decode it, and write to output_file.
    """
    byte_number = -1
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


def main():
    """main driver"""
    operation = sys.argv[1] if len(sys.argv) > 1 else "decode"
    input_file = (
        sys.argv[2]
        if len(sys.argv) > 2
        else ("msg.txt" if operation == "encode" else "processed.bin")
    )
    output_file = (
        sys.argv[3]
        if len(sys.argv) > 3
        else ("processed.bin" if operation == "encode" else "received.txt")
    )
    if operation == "encode":
        file_encoder(input_file, output_file)
    elif operation == "decode":
        file_decoder(input_file, output_file)
    else:
        print("Invalid operation. Use 'encode' or 'decode'.")
        sys.exit(1)


if __name__ == "__main__":
    main()
