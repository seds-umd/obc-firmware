# Modified from: https://gist.github.com/Lauszus/6c787a3bc26fea6e842dfb8296ebd630
# Setting should match crc32.c
def crc_poly(data, n=32, poly=0x04C11DB7, crc=0xFFFFFFFF):
    g = 1 << n | poly  # Generator polynomial

    # Loop over the data
    for d in data:
        # XOR the top byte in the CRC with the input byte
        crc ^= d << (n - 8)

        # Loop over all the bits in the byte
        for _ in range(8):
            # Start by shifting the CRC, so we can check for the top bit
            crc <<= 1

            # XOR the CRC if the top bit is 1
            if crc & (1 << n):
                crc ^= g

    # Return the CRC value
    return crc


def main():
    with open("application.bin", "rb") as f:
        application = f.read()

    # Fill with 1s as default state of flash
    header = bytearray([0xFF for _ in range(4096)])

    # Header format: https://github.com/seds-umd/obc-firmware/blob/main/docs/updater.md

    # Application size
    header[0:4] = len(application).to_bytes(4, "little")

    # Application checksum
    header[4:8] = crc_poly(application).to_bytes(4, "little")

    # Valid byte
    header[-1] = 0

    with open("header.bin", "wb") as f:
        f.write(header)


if __name__ == "__main__":
    main()
