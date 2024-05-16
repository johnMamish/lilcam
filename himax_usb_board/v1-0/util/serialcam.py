#!/usr/bin/python3
import serial
import time
import numpy as np
import cv2
import argparse

from camera_command_pb2 import *
from camerainterface import *

def establish_serial_connection(port):
    while True:
        try:
            ser = serial.Serial(port)
            return ser
        except serial.SerialException:
            print(f"Unable to connect to {port}. Retrying...")
            time.sleep(1)

def read_image_from_serial(args):
    ser = establish_serial_connection(args.port)
    camera = CameraInterface(ser)
    print(f"framesize = {camera.get_framesize()}")

    # Setup camera configuration
    camera.halt_dcmi()

    camera.disable_autoexposure()

    if (args.camera_select == "hm01b0"):
        camera.select_hm01b0()
    elif (args.camera_select == "hm0360"):
        camera.select_hm0360()

    camera.set_image_crop(2, 2, args.width, args.height)

    if (args.analog_gain is not None):
        camera.set_analog_gain(args.analog_gain)

    if (args.digital_gain is not None):
        camera.set_digital_gain(args.digital_gain)

    if (args.exposure is not None):
        camera.set_exposure(args.exposure)

    camera.force_command_update()

    # Perform manual register writes
    for regwrite in args.write_registers:
        if (len(regwrite.split(':')) != 2):
            raise ValueError(f"{regwrite} is a malformed register write string.")
        reg = int(regwrite.split(':')[0], 16)
        write = int(regwrite.split(':')[1], 16)
        print(f"Writing value {write:02x} to register {reg:04x} of image sensor.")
        camera.write_i2c_register(reg, write)

    # Start dcmi back up
    camera.resume_dcmi()

    saved_frame_count = 0

    while True:
        try:
            camera.try_read_bytes()

        except serial.SerialException:
            # TODO: re-establish connection
            raise

        if (camera.frame_ready()):
            frame = camera.pop_frame()

            # Display frame
            image_array = cv2.resize(frame,
                                     (args.width * args.upscale, args.height * args.upscale),
                                     interpolation=cv2.INTER_NEAREST)
            cv2.imshow('Serial Image', image_array.astype('uint8'))
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

            # Save file
            if (args.output is not None):
                base_filename = f"{args.output}_{saved_frame_count:04d}"
                if (args.filetype == "npy"):
                    with open(f"{base_filename}.npy", "wb") as f: np.save(f, frame)
                else:
                    cv2.imwrite(f"{base_filename}.{args.filetype}", frame)

            saved_frame_count += 1

        if ((args.nframes is not None) and (saved_frame_count >= args.nframes)):
            break

    camera.halt_dcmi()
    ser.close()
    cv2.destroyAllWindows()

def main():
    parser = argparse.ArgumentParser(description=
                                     'Read and display image data from a serial port.'
                                     '\n'
                                     "Note that for parameters that analog/digital gain and exposure which "
                                     "map to specific bit-fields in a register, the supplied values will be "
                                     "properly aligned. E.g. the hm01b0's analog gain is in bits [6:4] of "
                                     "register 0x0205. Just supplying the value '3' is enough to set this "
                                     "field. You should not supply the value (3 << 4) = 48.")
    parser.add_argument('port',
                        help='The name of the serial port to read from, eg /dev/ttyACM0')

    # Image sensor interface size
    parser.add_argument('--width', type=int, default=320,
                        help='Width of the image read from the sensor. Binning may affect this parameter.')
    parser.add_argument('--height', type=int, default=240,
                        help='Height of the image read from the sensor. Binning may affect this parameter')

    # TODO: add cropping args


    # TODO: add HM0360 selection option
    parser.add_argument('--camera-select', type=str, default='hm01b0', choices=['hm01b0', 'hm0360'],
                        help="Select image sensor to read from. Can be hm01b0 or hm0360.")

    # File saving options
    parser.add_argument('--nframes', type=int, default=None,
                        help='Number of frames to read before halting. '
                        'If no value is specified, then frames are read indefinately.')
    parser.add_argument('--output', type=str, default=None,
                        help="Prefix filename for output files. All frames will be prefixed with "
                        "this filename.")
    parser.add_argument('--filetype', type=str, default='npy',
                        help="File extension to use for saving files. "
                        "Supports npy as well as whatever cv2.imwrite() supports "
                        "(jpg, png, tiff, pgm, etc.)")

    # Image sensor config control
    parser.add_argument('--analog-gain', type=int, default=None,
                        help='Raw analog gain setting for image sensor. Refer to datasheet '
                        'for legal values and value meanings.')
    parser.add_argument('--digital-gain', type=int, default=None,
                        help='Raw digital gain setting for image sensor. Refer to datasheet '
                        'for legal values and value meanings.')
    parser.add_argument('--exposure', type=int, default=None,
                        help="Exposure time for image sensor. Refer to datasheet for legal values "
                        "and value meanings.")
    parser.add_argument('--write_registers', nargs='+', type=str, default=[],
                        help="List of register write commands to raw image sensors registers. "
                        "List should be supplied in the following format: \n"
                        "    <register_addr>:<register_value> <register_addr>:<register_value> ... \n"
                        "All values should be in hex. E.g. to write 0x02 and 0x1c to registers 0x020e "
                        "and 0x020f respectively, you'd do:\n"
                        "    0x020e:0x02 0x02f:0x1c\n"
                        "Note that the '0x' is optional.")

    # Display options
    parser.add_argument('--upscale', type=int, default=2,
                        help='Upscale factor for the image for display.')

    args = parser.parse_args()

    read_image_from_serial(args)


if __name__ == "__main__":
    main()
