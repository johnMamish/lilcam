import serial
import time
import numpy as np
import cv2
import argparse

def establish_serial_connection(port):
    while True:
        try:
            ser = serial.Serial(port)
            return ser
        except serial.SerialException:
            print(f"Unable to connect to {port}. Retrying...")
            time.sleep(1)

def read_image_from_serial(port, width, height):
    ser = establish_serial_connection(port)
    total_pixels = width * height
    chunk_size = 64  # Adjust this based on your needs

    last_time = time.time()
    total_bytes_read = 0

    while True:
        image_data = []

        while len(image_data) < total_pixels:
            try:
                current_time = time.time()
                bytes_to_read = min(ser.in_waiting, chunk_size)
                if bytes_to_read >= 2:
                    data = ser.read((bytes_to_read >> 1) << 1)
                    total_bytes_read += len(data)
                    for i in range(0, len(data), 2):
                        if i+1 < len(data):
                            ms_nibble = data[i] & 0x0F
                            ls_nibble = data[i+1] & 0x0F
                            pixel = (ms_nibble << 4) | ls_nibble
                            image_data.append(pixel)

                    # Print data rate every second
                    if current_time - last_time >= 1:
                        data_rate = total_bytes_read / (current_time - last_time)
                        print(f"Current data rate: {data_rate:.2f} bytes/second")
                        last_time = current_time
                        total_bytes_read = 0

            except serial.SerialException:
                print("Serial connection lost. Attempting to reconnect...")
                ser = establish_serial_connection(port)
                image_data = []  # Reset image data as we might have partial data
                last_time = time.time()  # Reset time
                total_bytes_read = 0
                continue

        if len(image_data) == total_pixels:
            image_array = np.array(image_data).reshape((height, width))
            cv2.imshow('Serial Image', image_array.astype('uint8'))
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    ser.close()
    cv2.destroyAllWindows()

def main():
    parser = argparse.ArgumentParser(description='Read and display image data from a serial port.')
    parser.add_argument('port', help='The name of the serial port to read from.')
    parser.add_argument('--width', type=int, default=320, help='Width of the image (default: 320).')
    parser.add_argument('--height', type=int, default=240, help='Height of the image (default: 240).')

    args = parser.parse_args()

    read_image_from_serial(args.port, args.width, args.height)

if __name__ == "__main__":
    main()
