import serial
import time
import numpy as np
from enum import Enum, auto
#from camera_command_pb2.pb_camera_management_request_sensor_select import sensor_select_e as sensor_select_e
from camera_command_pb2 import *
from collections import deque

class CameraInterface:
    """
    Utility class for configuring the camera.
    Keeps track of some basic state.

    TODO: we should also include reading in this class.
    """

    def __init__(self, serial):
        # data state variables
        self.frame_queue = deque()
        self.image_data = b''

        # serial comm params
        self.CHUNK_SIZE = (1 << 12)
        self.serial = serial

        # camera config state
        self.cameratype = pb_camera_management_request_sensor_select.sensor_select_e.HM01B0
        self.dcmi_halted = True
        self.packed = True
        self.cropdims = [2, 2, 320, 240]    # start_x, start_y, width, height

        # data rate telemetry
        self.MA_RATE = 0.05
        self.MA_WINDOW_LENGTH = 10
        self.data_rate_buffer = [0] * self.MA_WINDOW_LENGTH
        self.frame_rate_buffer = [0] * self.MA_WINDOW_LENGTH
        self.dt_buffer = [0] * self.MA_WINDOW_LENGTH
        self.frame_count = 0
        self.total_frames_decoded = 0
        self.total_bytes_read = 0
        self.last_time = time.time()


    ################################################################
    ###      Configuration methods
    ################################################################

    def __write_serial_with_prefix(self, msg):
        """
        This system assumes all protobufs are prefixed with a 1-byte message length.
        This function will append that message length and then transmit.
        """
        self.serial.write((len(msg) & 0xff).to_bytes(1, 'little'))
        self.serial.write(msg)

    def __get_i2c_addr(self):
        return 0x24 if (self.cameratype == pb_camera_management_request_sensor_select.sensor_select_e.HM01B0) else 0x35

    def __set_image_packing(self, do_pack):
        msg = pb_camera_request(
            dcmi_config=pb_camera_read_request(
                pack=pb_camera_read_request_set_packing(
                    pack=do_pack
                )
            )
        )

        self.__write_serial_with_prefix(msg.SerializeToString())

    def set_image_crop(self, start_x, start_y, width, height):
        self.cropdims = [start_x, start_y, width, height]

        # Note that if packing is enabled (HM01B0 is selected), then the dcmi interface width is
        # 2x the image width
        dcmi_startx = start_x * 2 if (self.packed) else start_x
        dcmi_width = width * 2 if (self.packed) else width

        msg = pb_camera_request(
            dcmi_config=pb_camera_read_request(
                crop=pb_camera_read_request_set_crop(
                    start_x=dcmi_startx,
                    start_y=start_y,
                    len_x = dcmi_width,
                    len_y = height
                )
            )
        )

        self.__write_serial_with_prefix(msg.SerializeToString())

    def __select_image_sensor(self, cameratype):
        """
        Tells the camera to select an image sensor.
        The DCMI interface should be halted before calling this function.
        """
        msg = pb_camera_request(
            camera_management=pb_camera_management_request(
                sensor_select=pb_camera_management_request_sensor_select(
                    sensor_select=cameratype
                )
            )
        )

        self.__write_serial_with_prefix(msg.SerializeToString())

        if (self.cameratype == pb_camera_management_request_sensor_select.sensor_select_e.HM01B0):
            self.__set_image_packing(True)
        else:
            self.__set_image_packing(False)

    def select_hm01b0(self):
        self.__select_image_sensor(pb_camera_management_request_sensor_select.sensor_select_e.HM01B0)

    def select_hm0360(self):
        self.__select_image_sensor(pb_camera_management_request_sensor_select.sensor_select_e.HM0360)

    def write_i2c_register(self, register_addr, value):
        # Create a reg_write request
        reg_write_request = pb_camera_management_request_reg_write()
        reg_write_request.i2c_peripheral_address = self.__get_i2c_addr()
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
        self.__write_serial_with_prefix(serialized_data)

    def __set_dcmi_state(self, do_halt):
        """
        If 'halt' is true, halts dcmi. If 'halt' is false, starts dcmi.
        """
        # Create a dcmi request
        msg = pb_camera_request(
            dcmi_config=pb_camera_read_request(
                dcmi_halt=pb_camera_read_request_dcmi_enable(halt=do_halt)
            )
        )

        self.__write_serial_with_prefix(msg.SerializeToString())

    def halt_dcmi(self):
        self.__set_dcmi_state(True)

    def resume_dcmi(self):
        self.__set_dcmi_state(False)

    ################################################################
    ### sensor-specific commands
    ################################################################
    def force_command_update(self):
        if (self.cameratype == pb_camera_management_request_sensor_select.sensor_select_e.HM01B0):
            self.write_i2c_register(0x0104, 1)
        else:
            pass

    def disable_autoexposure(self):
        if (self.cameratype == pb_camera_management_request_sensor_select.sensor_select_e.HM01B0):
            self.write_i2c_register(0x2100, 0)
        else:
            pass

    def enable_autoexposure(self):
        if (self.cameratype == pb_camera_management_request_sensor_select.sensor_select_e.HM01B0):
            self.write_i2c_register(0x2100, 1)
        else:
            pass

    def set_analog_gain(self, gain):
        if (self.cameratype == pb_camera_management_request_sensor_select.sensor_select_e.HM01B0):
            self.write_i2c_register(0x0205, (gain << 4))
        else:
            pass

    def set_digital_gain(self, gain):
        if (self.cameratype == pb_camera_management_request_sensor_select.sensor_select_e.HM01B0):
            self.write_i2c_register(0x020e, ((gain >> 6) & 0x03))
            self.write_i2c_register(0x020f, ((gain & 0x3f) << 2))
        else:
            pass

    def set_exposure(self, exp):
        pass

    ################################################################
    ### Camera read functions
    ################################################################
    def __align_preamble(self, arr, preamble):
        """
        This function checks to see if the preamble is in the current array.
        If it isn't, the array is discarded.
        If it is, the preamble is aligned to the start of the array.
        """
        if (arr[0:len(preamble)] == preamble):
            return arr

        for i in range(len(arr) - len(preamble) + 1):
            if (arr[i:(i + len(preamble))] == preamble):
                return arr[i:]

        return b''

    def get_imagesize(self):
        return self.cropdims[2] * self.cropdims[3]

    def get_framesize(self):
        return self.get_imagesize() + len(self.PREAMBLE)

    def try_read_bytes(self):
        """
        This function should be called in a loop.
        Once enough bytes have been read, you can call pop_frame to get a byte buffer of
        camera data.
        """
        try:
            # read serial data
            current_time = time.time()
            bytes_to_read = min(self.serial.in_waiting, self.CHUNK_SIZE)
            data = self.serial.read(bytes_to_read)
            self.total_bytes_read += len(data)
            self.image_data = self.image_data + data

            # update data rate telemetry
            if ((current_time - self.last_time) >= self.MA_RATE):
                self.data_rate_buffer.append(self.total_bytes_read)
                self.frame_rate_buffer.append(self.total_frames_decoded)
                self.dt_buffer.append(current_time - self.last_time)

                self.data_rate_buffer.pop(0)
                self.frame_rate_buffer.pop(0)
                self.dt_buffer.pop(0)

                self.last_time = current_time
                self.total_frames_decoded = 0
                self.total_bytes_read = 0

        except serial.SerialException:
            self.image_data = []  # Reset image data as we might have partial data
            self.last_time = time.time()  # Reset time
            self.total_bytes_read = 0
            self.total_frames_decoded = 0
            self.data_rate_buffer = [0] * self.MA_WINDOW_LENGTH
            self.frame_rate_buffer = [0] * self.MA_WINDOW_LENGTH
            raise

        self.image_data = self.__align_preamble(self.image_data, bytes(self.PREAMBLE[0:32]))

        # If we decoded a full frame, convert it to numpy and add it to our queue of images.
        if ((len(self.image_data) >= self.get_framesize()) and
            (self.image_data[0:32] == bytes(self.PREAMBLE[0:32]))):
            image_data_no_preamble = self.image_data[len(CameraInterface.PREAMBLE):]
            image_array = np.frombuffer(image_data_no_preamble[0:self.get_imagesize()], dtype=np.uint8) \
                                        .reshape((self.cropdims[3], self.cropdims[2]))
            self.frame_queue.append(image_array)
            self.image_data = self.image_data[self.get_framesize():]
            self.total_frames_decoded += 1

    def get_frame_rate(self):
        return sum(self.frame_rate_buffer) / sum(self.dt_buffer)

    def get_data_rate(self):
        return sum(self.data_rate_buffer) / sum(self.dt_buffer)

    def pop_frame(self):
        """
        If a frame is available, pops it.
        Returns a single numpy array
        """
        return self.frame_queue.popleft()

    def frame_ready(self):
        """
        Returns True if there's a fully prepared frame.
        """
        return len(self.frame_queue) > 0

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
