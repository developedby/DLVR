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
        self.ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=20)
        time.sleep(2)

    def close(self):
        self.ser.close()

    def write(self, packet_to_send):
        packet_to_send.append(self.START_BYTE)
        packet_to_send.insert(0, self.START_BYTE)
        print("mandando para o radio o pacote", packet_to_send)
        self.ser.write(packet_to_send)

    def read(self):
        byte = self.ser.read()
        received_packet = []
        start_byte_byte = self.START_BYTE.to_bytes(1, byteorder='big')
        if byte == start_byte_byte:
            received_packet = self.ser.read_until(start_byte_byte, 50)
            if received_packet[-1] != self.START_BYTE:
                received_packet = []
        print("pacote recebido do robo")
        for a in received_packet:
            print(a, end = ' ')
        print("")
        return received_packet

class VehicleServerInterface:
    #dictionaries to vehicle
    vehicle_sensors_code = {"item detector" : 1, "ultrasound" : 2, "lock" : 3}
    possible_commands = {"force_go_ahead" : 1, "open_box" : 2, "close_box" : 3, "get qr_code" : 4}
    possible_status_to_vehicle = {"movement" : 1, "status_robot" : 2}
    directions_to_vehicle = {"back" : 2, "right": 6, "left": 4, "front" : 8}
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
            dict_to_server["path"] = "/robot/update"
            dict_to_server["id"] = 0
            dict_to_server["timestamp"] = datetime.datetime.now().timestamp()
        else:
            dict_to_server = {"path" : "/robot/update", "id": 0, "timestamp" : datetime.datetime.now().timestamp(), "state" : 10}
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
            remove_last_step = not ("add_last_path" in msg)
            route, qr_code_destination = self.decodePathFromServer(msg["route"], remove_last_step)
        if("command" in msg):
            command = msg["command"]
            print("comando eh: ", command)
            if(command == "get qr_code"):
                qr_code_destination = msg["qr"]
                print("vou achar o qr code ", qr_code_destination)
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
            if(command == "get qr_code"):
                packet_to_vehicle.append((qr_code_destination & 0xff00) >> 8)
                packet_to_vehicle.append((qr_code_destination & 0xff))
        if sensor_to_read:
            packet_to_vehicle.append(self.vehicle_sensors_code[sensor_to_read])
        if required_status:
            packet_to_vehicle.append(self.possible_status_to_vehicle[required_status])
        #print(packet_to_vehicle)
        return packet_to_vehicle, sensor_to_read, required_status

    def decodeMessageFromVehicle(self, msg, sensor_to_read, required_status):
        current_address = 1
        sensor_read = 0
        status = 0
        qr_codes_read = []
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
            #status = self.possible_status_from_vehicle[msg[current_address]]
            status = msg[current_address]
            dict_from_vehicle["state"] = int(status)
            current_address += 1
        print(current_address)
        qr_codes_read.extend(msg[current_address : -1])
        qr_codes_read_int = []
        if((len(qr_codes_read) > 1) and (len(qr_codes_read) % 2 == 0)):
            for i in range(0, len(qr_codes_read), 2):
                a = qr_codes_read[i] << 8
                a += qr_codes_read[i + 1]
                qr_codes_read_int.append(a)
        if qr_codes_read_int:
            dict_from_vehicle["qr"] = qr_codes_read_int[-1]
            print("qr code eh: ", qr_codes_read_int[-1])
        return dict_from_vehicle

    def decodePathFromServer(self, path, remove_last_step=True):
        decoded_path = []
        qr_code_destination = 0
        last_position = len(path) - 1 if remove_last_step else len(path)
        for i in range(0, last_position):
            decoded_path.append(self.directions_to_vehicle[path[i][0]])
            decoded_path.append(int(path[i][1] * 100))
        if not remove_last_step:
            decoded_path[-1] -= 140
        qr_code_destination = path[-1][2]
        return decoded_path, qr_code_destination


vehicle_server = VehicleServerInterface()

def updateServer():
    msg = {"required_status" : "status_robot", "sensor_to_read": "item detector"}
    #data = vehicle_server.handle_server_request(1, msg)
    data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
    return data



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
            print("Signin done successfully")
            msg = {"required_status" : "status_robot"}
            #data = vehicle_server.handle_server_request(1, msg)
            data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
            print("data inicial: ",  data)
            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
            await websocket.send(json.dumps(data))
            while(not "qr" in data):
                print("robo nao esta reconhecendo o qr code da cidade")
                data = await loop.run_in_executor(None, updateServer)
                data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                await websocket.send(json.dumps(data))
                await asyncio.sleep(1)
            print("esperando mensagem do servidor")
            async for message in websocket:
                print(message)
                resp = json.loads(message)
                if "path" in resp:
                    if resp["path"] == "/delivery/response": #tem o caminho ate o inicio da entrega
                        print("Iniciando delivery")
                        msg = {}
                        if("path" in resp["message_body"]):
                            route = resp["message_body"]["path"]
                            msg["route"] = route
                            print("tem uma rota")
                        msg["command"] = "close_box"
                        msg["required_status"] = "status_robot"
                        #data = vehicle_server.handle_server_request(1, msg)
                        data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        while(data["state"] != 2):
                            data = await loop.run_in_executor(None, updateServer)
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            await asyncio.sleep(1)
                        while(data["state"] != 1):
                            data = await loop.run_in_executor(None, updateServer)
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            await asyncio.sleep(1)
                        msg = {"command" : "open_box"}
                        #data = vehicle_server.handle_server_request(1, msg)
                        data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        print("Container aberto")
                    elif resp["path"] == "/delivery/send":#tem o caminho para o local de entrega
                        print("verificando item")
                        msg = {"required_status" : "status_robot", "sensor_to_read": "item detector"}
                        #data = vehicle_server.handle_server_request(1, msg)
                        data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        while(data["item"] != 1):
                            print("esperando colocar item")
                            data = await loop.run_in_executor(None, updateServer)
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            await asyncio.sleep(1)
                        print("fechando caixa")
                        msg = {}
                        if("path" in resp["message_body"]):
                            route = resp["message_body"]["path"]
                            msg["route"] = route
                            print("tem uma rota")
                        msg["command"] = "close_box"
                        msg["required_status"] = "status_robot"
                        #data = vehicle_server.handle_server_request(1, msg)
                        data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        while(data["state"] != 2):
                            data = await loop.run_in_executor(None, updateServer)
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            await asyncio.sleep(1)
                        while(data["state"] != 1):
                            data = await loop.run_in_executor(None, updateServer)
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            await asyncio.sleep(1)
                    elif resp["path"] == "/delivery/qr": #tem o qr code para ver se o valor esta correto
                        print("QR code read")
                        msg = {"command" : "get qr_code"}
                        msg["qr"] = resp["message_body"]
                        msg["required_status"] = "status_robot"
                        #data = vehicle_server.handle_server_request(1, msg)
                        data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        stop = False
                        if "qr" in data and data["qr"] == msg["qr"]:
                            stop = True
                        while(not stop):
                            print("esperando reconhecer qr code usuario")
                            msg = {"command" : "get qr_code"}
                            msg["qr"] = resp["message_body"]
                            msg["required_status"] = "status_robot"
                            #data = vehicle_server.handle_server_request(1, msg)
                            data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            if "qr" in data and data["qr"] == msg["qr"]:
                                stop = True
                        print("Container opened")

                    elif resp["path"] == "/delivery/finish": #tem o caminho para a garagem
                        print("Delivery finished")
                        print("verificando item")
                        msg = {"required_status" : "status_robot", "sensor_to_read": "item detector"}
                        #data = vehicle_server.handle_server_request(1, msg)
                        data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        while(data["item"] == 1):
                            print("esperando tirar o item da caixa")
                            data = await loop.run_in_executor(None, updateServer)
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            await asyncio.sleep(1)
                        print("fechando caixa")
                        msg = {}
                        if("path" in resp["message_body"]):
                            route = resp["message_body"]["path"]
                            msg["route"] = route
                            print("tem uma rota com um cara a menos")
                            msg["add_last_path"] = True
                        msg["command"] = "close_box"
                        msg["required_status"] = "status_robot"
                        #data = vehicle_server.handle_server_request(1, msg)
                        data = await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        while(data["state"] != 2):
                            data = await loop.run_in_executor(None, updateServer)
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            await asyncio.sleep(1)
                        while(data["state"] != 1):
                            data = await loop.run_in_executor(None, updateServer)
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                            await asyncio.sleep(1)
                        print("delivery finished")
                        #data = {"path": "/robot/signout", "id": 0, "timestamp": datetime.datetime.now().timestamp()}
                        #data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        #await websocket.send(json.dumps(data))
                        #return
                    #tests
                    elif resp["path"] == "/debug/open":
                        msg = {"command" : "open_box"}
                        #vehicle_server.handle_server_request(1, msg)
                        await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 2, "qr": 14}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        print("Container aberto")
                    elif resp["path"] == "/debug/close":
                        print("Container fechado")
                        msg = {"command" : "close_box"}
                        #vehicle_server.handle_server_request(1, msg)
                        await loop.run_in_executor(None, lambda: vehicle_server.handle_server_request(1, msg))
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 3}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
        else:
            print(resp)
            print("Signin incorreto")

if __name__ == "__main__":
    main()
