CHUNK_SIZE_BYTE = 7
MAGIC_BIT_SEQ = 0xAA  # 10101010


def file_encoder(input_file: str, output_file: str):
    """
    Read input_file byte by byte, encode it, and write to output_file.
    """
    byte_number = 0
    with open(input_file, "rb") as infile, open(output_file, "wb") as outfile:
        while True:
            if (byte_number) % CHUNK_SIZE_BYTE == 0:
                outfile.write(MAGIC_BIT_SEQ.to_bytes(1, byteorder="big"))
            byte = infile.read(1)
            byte_number += 1
            if not byte:
                break
            outfile.write(byte)


def file_decoder(input_file: str, output_file: str):
    """
    Read input_file byte by byte, decode it, and write to output_file.
    """
    byte_number = -1
    with open(input_file, "rb") as infile, open(output_file, "wb") as outfile:
        while True:
            byte = infile.read(1)
            if not byte:
                break
            byte_number += 1
            if byte_number % (CHUNK_SIZE_BYTE + 1) == 0:
                continue
            outfile.write(byte)


file_encoder("msg.txt", "processed.bin")
file_decoder("processed.bin", "received.txt")
