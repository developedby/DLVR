import requests
import serial
import struct
import time
from datetime import datetime
import threading
import urllib.request
import ssl
import json

class SerialInterface:
    START_BYTE = 0Xaa
    url_request = ""
    url_post = ""

    def __init__(self):
        self.ser = None
        self.open()

    def __del__(self):
        self.close()

    def open(self):
        self.ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=5)
        time.sleep(2)

    def close(self):
        self.ser.close()

    def test_echo(self):
        #server
        #variables from server
        route = [1, 1, 2, 2, 3, 3, 4, 4]
        qr_code_destination = 0x0005
        required_status = "status_robot"
        device_id = 1
        sensor_to_read = "ultrasound"
        command = "open_box"
        #variables to server
        qr_codes_read = bytearray()
        sensor_reading = 0
        status = ""
        #dictionaries to server
        movement = {"amplitude" : 0.00, "curvature" : 0.00}

        #vehicle
        #dictionaries to vehicle
        vehicle_sensors_code = {"item detector" : 1, "ultrasound" : 2, "lock" : 3}
        possible_commands = {"force_go_ahead" : 1, "open_box" : 2, "close_box" : 3, "get qr_code" : 4}
        possible_status_to_vehicle = {"movement" : 1, "status_robot" : 2}
        #dictionaries from vehicle
        possible_status_from_vehicle = {1 : "stopped", 2 : "moving forward", 3 :"moving backway", 4 : "avoiding obstacle", 5 : "waiting semaphore", 6 : "waiting route"}
        #packet serial
        packet_to_vehicle = []
        packet_from_vehicle = []
        radio_address = "DLVR1"

        #build packet to vehicle
        packet_to_vehicle.append(START_BYTE)
        packet_to_vehicle.extend(list(radio_address.encode("ascii")))
        packet_to_vehicle.append(device_id)
        packet_content = (1 if route else 0) << 3
        packet_content += (1 if command else 0) << 2
        packet_content += (1 if sensor_to_read else 0) << 1
        packet_content += (1 if required_status else 0) << 0
        packet_to_vehicle.append(packet_content)
        if route:
            packet_to_vehicle.append(len(route))
            packet_to_vehicle.extend(route)
            packet_to_vehicle.append((qr_code_destination & 0xff00) >> 8)
            packet_to_vehicle.append((qr_code_destination & 0xff))
        if command:
            packet_to_vehicle.append(possible_commands[command])
        if sensor_to_read:
            packet_to_vehicle.append(vehicle_sensors_code[sensor_to_read])
        if required_status:
            packet_to_vehicle.append(possible_status_to_vehicle[required_status])

        packet_to_vehicle.append(START_BYTE)
        print(packet_to_vehicle)
        packet_to_vehicle = bytes(packet_to_vehicle)
        print(type(packet_to_vehicle), len(packet_to_vehicle), packet_to_vehicle)
        self.ser.write(packet_to_vehicle)

        byte = self.ser.read()
        start_byte_byte = START_BYTE.to_bytes(1, byteorder='big')
        if byte == start_byte_byte:
            print("legal")
            packet_from_vehicle = self.ser.read_until(start_byte_byte, 50)
        #packet_from_vehicle = self.ser.readall()
        print(type(packet_from_vehicle), len(packet_from_vehicle), packet_from_vehicle)


        #interpret packet from vehicle

        if (packet_from_vehicle and packet_from_vehicle[-1] == START_BYTE
                and packet_from_vehicle[0] == device_id):
            current_address = 1
            if sensor_to_read == "ultrasound":
                sensor_read = struct.unpack('f', packet_from_vehicle[current_address:(current_address+4)])
                current_address += 4
            else:
                    sensor_read = packet_from_vehicle[current_address]
                    current_address += 1
            if required_status == "movement":
                movement["amplitude"] = struct.unpack('f', packet_from_vehicle[current_address:(current_address+4)])
                current_address += 4
                movement["curvature"] = struct.unpack('f',packet_from_vehicle[current_address:(current_address+4)])
                current_address += 4
                status = movement
            elif required_status == "status_robot":
                status = possible_status_from_vehicle[packet_from_vehicle[current_address]]
                current_address += 1
            qr_codes_read += packet_from_vehicle[current_address : -1]
            print("sensor: ", sensor_read)
            print("status: ", status)
            print(list(qr_codes_read))
        print("fim")

class ServerInterface:
    """Sends and receives stuff from the server
    Handles multiple vehicles in one instance
    """
    def __init__(self, address, callback_server_msg):
        self.address = address
        self.connections = {}
        self.callback_server_msg = callback_server_msg

    def send_json(self, url, data_dict = None, method = "POST", cookie = None):
        headers = {}
        data = None
        if data_dict:
            data = json.dumps(data_dict).encode("utf-8")
            headers["Content-Type"] = "application/json"
        if cookie:
            headers["Cookie"] = cookie
        req = urllib.request.Request(self.address+url, data,
                                     headers=headers, method=method)
        context = ssl._create_unverified_context()
        resp = urllib.request.urlopen(req, timeout=30, context=context)
        return resp

    def send_update(self, packet):
        self.send_json("/robot/update", packet)

    def request_new_route(self, robot_id):
        self.send_json("/robot/route", {'signature': robot_id})

    def sign_in(self, robot_id):
        resp = self.send_json("/robot/signin", {'signature': robot_id,
                                                'timestamp': str(datetime.now())})
        self.connections[robot_id] = threading.Thread(target=self.receive_from_server,
                                                      args=(robot_id, resp))

    def receive_from_server(self, robot_id, resp):
        connection_active = True
        while connection_active:
            msg = resp.read().decode('utf-8')
            self.callback_server_msg(robot_id, msg)

    def sign_out(self, robot_id):
        self.send_json("robot/signout", {'signature': robot_id})
    

class VehicleServerInterface:
    def __init__(self, serial_port, server_address):
        self.server_interface = ServerInterface(server_address, self.handle_server_request)
        self.serial_interface = SerialInterface()

    def handle_server_request(self, robot_id, msg):
        pass

    def run(self):
        while True:
            pass
