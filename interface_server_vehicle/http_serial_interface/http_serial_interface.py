#!/usr/bin/env python3

import json
import datetime
import hashlib
import Crypto.PublicKey.RSA
import asyncio
import websockets
import ssl
import struct
import serial
import time

CERT_PEM_PATH = './cert.pem'
ROBOT_DER_PATH = './robot0.der'
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
ssl_context.load_verify_locations(CERT_PEM_PATH)
loop = asyncio.get_event_loop()

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
    
    def write(self, packet_to_send):
        packet_to_send.append(self.START_BYTE)
        packet_to_send.insert(0, self.START_BYTE)
        self.ser.write(packet_to_send)

    def read(self):
        byte = self.ser.read()
        received_packet = []
        start_byte_byte = self.START_BYTE.to_bytes(1, byteorder='big')
        if byte == start_byte_byte:
            received_packet = self.ser.read_until(start_byte_byte, 50)
            if received_packet[-1] != self.START_BYTE:
                received_packet = []
        return received_packet

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

class VehicleServerInterface:
    #dictionaries to vehicle
    vehicle_sensors_code = {"item detector" : 1, "ultrasound" : 2, "lock" : 3}
    possible_commands = {"force_go_ahead" : 1, "open_box" : 2, "close_box" : 3, "get qr_code" : 4}
    possible_status_to_vehicle = {"movement" : 1, "status_robot" : 2}
    #dictionaries from vehicle
    possible_status_from_vehicle = {1 : "stopped", 2 : "moving forward", 3 :"moving backway", 4 : "avoiding obstacle", 5 : "waiting semaphore", 6 : "waiting route"}
    
    def __init__(self):
        self.serial_interface = SerialInterface()
        
    def handle_server_request(self, robot_id, msg):
        packet_to_send, sensor_to_read, required_status = self.decodeMessageFromServer(robot_id, msg)
        self.serial_interface.write(packet_to_send)
        packet_received = self.serial_interface.read()
        if (packet_received and packet_received[0] == robot_id):
            dict_to_server = self.decodeMessageFromVehicle(packet_received, sensor_to_read, required_status)
            return dict_to_server

    def decodeMessageFromServer(self, robot_id, msg):
        packet_to_vehicle = []
        radio_address = "DLVR1"
        packet_to_vehicle.extend(list(radio_address.encode("ascii")))
        packet_to_vehicle.append(robot_id)
        route = ""
        command = ""
        sensor_to_read = ""
        required_status = ""
        qr_code_destination = 0
        if("route" in msg):
            route = msg["route"]
            qr_code_destination = msg["qr"]
        if("command" in msg):
            command = msg["command"]
        if("sensor_to_read" in msg):
            sensor_to_read = msg["sensor_to_read"]
        if("required_status" in msg):
            required_status = msg["required_status"]
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
            packet_to_vehicle.append(self.possible_commands[command])
        if sensor_to_read:
            packet_to_vehicle.append(self.vehicle_sensors_code[sensor_to_read])
        if required_status:
            packet_to_vehicle.append(self.possible_status_to_vehicle[required_status])
        return packet_to_vehicle, sensor_to_read, required_status

    def decodeMessageFromVehicle(self, msg, sensor_to_read, required_status, json):
        current_address = 1
        sensor_read = 0
        status = 0
        qr_codes_read = 0
        dict_from_vehicle = {}
        if sensor_to_read == "ultrasound":
            sensor_read = struct.unpack('f', msg[current_address:(current_address+4)])
            dict_from_vehicle["ultrasound"] = sensor_read
            current_address += 4
        else:
            sensor_read = msg[current_address]
            if sensor_to_read == "item detector":
                dict_from_vehicle["item"] = sensor_read
            else:
                pass
            current_address += 1
        if required_status == "movement":
            speed = struct.unpack('f', msg[current_address:(current_address+4)])
            current_address += 4
            curve_radius = struct.unpack('f',msg[current_address:(current_address+4)])
            current_address += 4
            dict_from_vehicle["speed"] = speed
            dict_from_vehicle["curve_radius"] = curve_radius
        elif required_status == "status_robot":
            status = self.possible_status_from_vehicle[msg[current_address]]
            dict_from_vehicle["state"] = status
            current_address += 1
        qr_codes_read += msg[current_address : -1]
        dict_from_vehicle["qr"] = qr_codes_read
        return dict_from_vehicle

        return sensor_read, status, qr_codes_read

vehicle_server = VehicleServerInterface()
def main():
    loop.run_until_complete(handler())

async def handler():
    uri = "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
    data = {"id": 0, "timestamp": datetime.datetime.now().timestamp()}
    with open(ROBOT_DER_PATH, "r") as file:
        private_key = Crypto.PublicKey.RSA.importKey(bytes.fromhex(file.read()))
    data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
    async with websockets.connect(uri + "/robot/signin", ssl = ssl_context) as websocket:
        await websocket.send(json.dumps(data))
        resp = await websocket.recv()
        resp = json.loads(resp)
        if resp["status_code"] == 200 and resp["message_body"] == "true":
            print("Signin efetuado com sucesso")
            async for message in websocket:
                print(message)
                resp = json.loads(message)
                if "path" in resp:
                    if resp["path"] == "/delivery/response": 
                        print("Iniciando delivery")
                        msg = []
                        """
                        if("path" in resp["message_body"]):
                            route = resp["message_body"]["path"]
                            qr_code_destination = resp["message_body"]["qr"]
                            msg["route] = route
                            msg["qr"] = qr_code_destination
                            print("tem uma rota")
                        vehicle_server.decodeMessageFromServer(1, msg)
                        """
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 1}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        msg = {"command" : "open_box"}
                        vehicle_server.handle_server_request(1, msg)
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 2, "qr": 14}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        print("Container aberto")
                    elif resp["path"] == "/delivery/send":
                        print("Container fechado")
                        msg = {"command" : "close_box"}
                        vehicle_server.handle_server_request(1, msg)
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 3}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 0, "qr": 0}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        data = {"path": "/robot/signout", "id": 0, "timestamp": datetime.datetime.now().timestamp()}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        print("logout")
                        return
        else:
            print(resp)
            print("Signin incorreto")

if __name__ == "__main__":
    main()
