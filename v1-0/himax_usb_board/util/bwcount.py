#!/usr/bin/python3

import argparse
import time
import serial

def parse_arguments():
    parser = argparse.ArgumentParser(description="Measure incoming bytes on a serial port.")
    parser.add_argument("port", type=str, help="Serial port to open (e.g., COM3 or /dev/ttyUSB0)")
    parser.add_argument("-t", "--time", type=float, default=1.0, help="Duration to keep the port open in seconds")
    return parser.parse_args()

def open_serial_port(port, timeout):
    try:
        return serial.Serial(port, timeout=timeout)
    except serial.SerialException as e:
        print(f"Error opening serial port {port}.")
        exit(1)

def count_bytes(ser, duration):
    start_time = time.time()
    end_time = start_time + duration
    byte_count = 0

    while time.time() < end_time:
        byte_count += len(ser.read(ser.in_waiting))

    return byte_count

def main():
    args = parse_arguments()
    ser = open_serial_port(args.port, args.time)

    print(f"Opened serial port {args.port}. Measuring for {args.time} seconds...")
    byte_count = count_bytes(ser, args.time)
    ser.close()

    bandwidth_mbps = byte_count / args.time / 1e6
    print(f"Total Bytes Received: {byte_count}")
    print(f"Bandwidth: {bandwidth_mbps:.3f} MB/s")

if __name__ == "__main__":
    main()
