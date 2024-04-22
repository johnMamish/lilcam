#!/usr/bin/python3
import serial
import time
import numpy as np
import cv2
import argparse

from camera_command_pb2 import *


def establish_serial_connection(port):
    while True:
        try:
            ser = serial.Serial(port)
            return ser
        except serial.SerialException:
            print(f"Unable to connect to {port}. Retrying...")
            time.sleep(1)

PREAMBLE = [
    0x48, 0xc1, 0x20, 0xa3, 0xb3, 0x94, 0x74, 0xc2, 0xa6, 0xb7,
    0xc7, 0x76, 0x1e, 0x07, 0x4c, 0x07, 0xde, 0x00, 0x46, 0x44,
    0x05, 0xc3, 0x96, 0xe6, 0xfe, 0x38, 0xc3, 0x2f, 0x1e, 0x5a,
    0x96, 0xb8, 0x8c, 0x4e, 0x1a, 0x2a, 0x9c, 0xc9, 0xf4, 0x67,
    0x29, 0xbf, 0x5c, 0x55, 0xe6, 0x25, 0xfa, 0x4a, 0xaa, 0x17,
    0xd0, 0x54, 0xaf, 0x0b, 0xd6, 0x4c, 0xdc, 0x8d, 0x63, 0x19,
    0x2f, 0x20, 0xcb, 0x5d, 0xd5, 0x7b, 0x45, 0x5a, 0xb7, 0x36,
    0x1e, 0xdb, 0x7f, 0xde, 0x6e, 0x5e, 0xc1, 0x2c, 0x42, 0x43,
    0x71, 0x97, 0x69, 0xb1, 0x6a, 0x82, 0x78, 0x2d, 0xab, 0xab,
    0xf6, 0x33, 0xef, 0x9f, 0x7f, 0x59, 0xa1, 0xbd, 0x65, 0x9e,
    0x91, 0x29, 0xdf, 0x20, 0x91, 0x07, 0x23, 0x32, 0x1d, 0x2a,
    0xf8, 0xeb, 0x95, 0xc0, 0xe9, 0x6a, 0x90, 0x53, 0x70, 0x89,
    0x11, 0xb3, 0x7b, 0x21, 0xb6, 0x7c, 0xb6, 0xc4, 0x80, 0x47,
    0xba, 0x52, 0x58, 0xcb, 0x35, 0x0a, 0x1b, 0x0a, 0x89, 0xe1,
    0x8c, 0xf7, 0xef, 0xc9, 0x13, 0x4a, 0x05, 0x48, 0x97, 0x0e,
    0x29, 0xf0, 0xc0, 0xcd, 0x15, 0xd7, 0x90, 0x08, 0x36, 0x86,
    0xee, 0x8e, 0x23, 0x37, 0xde, 0x0e, 0xe6, 0xee, 0x78, 0x61,
    0xfe, 0x42, 0xc8, 0xc4, 0x1d, 0x9f, 0xf3, 0xd2, 0x13, 0x98,
    0xc9, 0x34, 0x27, 0xc4, 0xd2, 0x12, 0x8c, 0x1a, 0xdd, 0xbf,
    0x28, 0xf0, 0xd3, 0xdd, 0xdf, 0x49, 0x90, 0x3e, 0xc6, 0xb0,
    0x5a, 0xd1, 0x5b, 0xbd, 0x92, 0xe1, 0xd4, 0xa1, 0x35, 0xe9,
    0x60, 0xbe, 0x79, 0x89, 0x28, 0x4e, 0xa0, 0xdd, 0xdc, 0xcb,
    0xc2, 0xb3, 0x7c, 0x6a, 0x48, 0xe2, 0x84, 0x57, 0x95, 0x56,
    0xbf, 0x34, 0xfb, 0x9f, 0xc3, 0x2b, 0x38, 0x57, 0x64, 0x73,
    0xd8, 0xf2, 0xc1, 0xee, 0x97, 0x61, 0x20, 0xf7, 0x26, 0xc1,
    0x4c, 0x69, 0xfb, 0xe2, 0xd2, 0xb3, 0x27, 0x0e, 0xa7, 0xa7,
    0xaf, 0xf9, 0x36, 0xec, 0xa7, 0x64, 0x2a, 0x69, 0xf7, 0xec,
    0x13, 0xdc, 0x53, 0xa3, 0x39, 0x64, 0x85, 0x0e, 0x1d, 0x10,
    0x49, 0x21, 0xe0, 0xc4, 0x9d, 0xdc, 0xaa, 0x11, 0xe6, 0xfc,
    0xca, 0xfe, 0x37, 0x92, 0x3f, 0xed, 0x4e, 0x0c, 0x69, 0x86,
    0x5f, 0x66, 0x59, 0xfc, 0x7a, 0x95, 0x5b, 0xcd, 0x18, 0xca,
    0x86, 0xac, 0xd4, 0xc8, 0xef, 0x8f, 0x89, 0x2b, 0xe6, 0x2d
]

def align_preamble(arr, preamble):
    """
    This function checks to see if the preamble is in the current array.
    If it isn't, the array is discarded.
    If it is, the preamble is aligned to the start of the array.
    """
    if (arr[0:len(preamble)] == preamble):
        return arr

    matchidx = 0
    for i in range(len(arr) - len(preamble) + 1):
        if (arr[i:(i + len(preamble))] == preamble):
            return arr[i:]
    return arr

def write_serial_with_prefix(serial, msg):
    """
    This system assumes all protobufs are prefixed with a 1-byte message length.
    This function will append that message length and then transmit.
    """
    serial.write((len(msg) & 0xff).to_bytes(1, 'little'))
    serial.write(msg)

def write_i2c_register(serial, i2c_addr, register_addr, value):
    # Create a reg_write request
    reg_write_request = pb_camera_management_request_reg_write()
    reg_write_request.i2c_peripheral_address = i2c_addr
    reg_write_request.register_address = register_addr
    reg_write_request.value = value

    # Embed this reg_write request into a camera_management request
    camera_management_request = pb_camera_management_request()
    camera_management_request.reg_write.CopyFrom(reg_write_request)

    # Now embed this camera_management request into the top-level pb_camera_request
    camera_request = pb_camera_request()
    camera_request.camera_management.CopyFrom(camera_management_request)

    # To serialize the message to a byte string (for sending over a network or writing to a file)
    serialized_data = camera_request.SerializeToString()
    print(' '.join([f"{x:02x}" for x in serialized_data]))
    print('\n')
    write_serial_with_prefix(serial, serialized_data)

def set_dcmi_halted(serial, halt):
    """
    If 'halt' is true, halts dcmi. If 'halt' is false, starts dcmi.
    """
    # Create a dcmi request
    msg = pb_camera_request(
        dcmi_config=pb_camera_read_request(
            dcmi_halt=pb_camera_read_request_dcmi_enable(halt=halt)
        )
    )

    write_serial_with_prefix(serial, msg.SerializeToString())

def read_image_from_serial(port, width, height, upscale_factor):
    ser = establish_serial_connection(port)
    imagesize = width * height
    framesize = width * height + len(PREAMBLE)
    chunk_size = (1 << 12)

    print(f"framesize = {framesize}")

    last_time = time.time()
    total_bytes_read = 0
    total_frames_decoded = 0

    image_data = b''

    # Change i2c and then start dcmi
    set_dcmi_halted(ser, True)

    # disable AE
    write_i2c_register(ser, 0x24, 0x2100, 0x00)
    write_i2c_register(ser, 0x24, 0x210e, 0x00)

    # set exposure level
    #write_i2c_register(ser, 0x24, 0x0205, (0 << 4))

    set_dcmi_halted(ser, False)

    # moving average params for monitoring FPS / data rate.
    MA_RATE = 0.05
    MA_WINDOW_LENGTH = 10
    data_rate_buffer = [0] * MA_WINDOW_LENGTH
    frame_rate_buffer = [0] * MA_WINDOW_LENGTH
    dt_buffer = [0] * MA_WINDOW_LENGTH
    frame_count = 0


    exp = 0

    while True:
        try:
            current_time = time.time()
            bytes_to_read = min(ser.in_waiting, chunk_size)
            data = ser.read(bytes_to_read)
            total_bytes_read += len(data)
            image_data = image_data + data

            # Print data rate every second
            if ((current_time - last_time) >= MA_RATE):
                data_rate_buffer.append(total_bytes_read)
                frame_rate_buffer.append(total_frames_decoded)
                dt_buffer.append(current_time - last_time)

                data_rate_buffer.pop(0)
                frame_rate_buffer.pop(0)
                dt_buffer.pop(0)

                data_rate = (sum(data_rate_buffer) / sum(dt_buffer)) / 1e6
                frame_rate = sum(frame_rate_buffer) / sum(dt_buffer)
                #print(f"reading at {data_rate:6.3f} MBps and {frame_rate:6.3f} fps", end='\r')
                last_time = current_time
                total_frames_decoded = 0
                total_bytes_read = 0

        except serial.SerialException:
            print("Serial connection lost. Attempting to reconnect...")
            ser = establish_serial_connection(port)
            image_data = []  # Reset image data as we might have partial data
            last_time = time.time()  # Reset time
            total_bytes_read = 0
            continue

        image_data = align_preamble(image_data, bytes(PREAMBLE[0:32]))

        if len(image_data) >= framesize:
            image_data_no_preamble = image_data[len(PREAMBLE):]
            #print(image_data_no_preamble)
            image_array = np.frombuffer(image_data_no_preamble[0:imagesize], dtype=np.uint8).reshape((height, width))
            image_array = cv2.resize(image_array, (width * upscale_factor, height * upscale_factor),
                                     interpolation=cv2.INTER_NEAREST)
            total_frames_decoded += 1
            cv2.imshow('Serial Image', image_array.astype('uint8'))
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

            image_data = image_data[framesize:]
            frame_count += 1

            if ((frame_count % 10) == 0):
                write_i2c_register(ser, 0x24, 0x0205, ((exp & 0x03) << 4))
                write_i2c_register(ser, 0x24, 0x0104, 1)

                exp += 1

    ser.close()
    cv2.destroyAllWindows()

def main():
    parser = argparse.ArgumentParser(description='Read and display image data from a serial port.')
    parser.add_argument('port', help='The name of the serial port to read from.')
    parser.add_argument('--width', type=int, default=320, help='Width of the image (default: 320).')
    parser.add_argument('--height', type=int, default=240, help='Height of the image (default: 240).')
    parser.add_argument('--upscale', type=int, default=2, help='Upscale factor for the image.')

    args = parser.parse_args()

    read_image_from_serial(args.port, args.width, args.height, args.upscale)


if __name__ == "__main__":
    main()
