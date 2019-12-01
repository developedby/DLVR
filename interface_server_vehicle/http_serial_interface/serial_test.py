import requests
import serial
import struct
import time

START_BYTE = 0Xaa
url_request = ""
url_post = ""

s = serial.Serial('/dev/ttyUSB0', 115200, timeout=10)
time.sleep(2)

#server
#variables from server
#route = [8, 72, 8, 26, 8, 72]
route = [8, 60, 6, 45, 6, 57]
#qr_code_destination = 0x0014
qr_code_destination = 0x0013
required_status = "status_robot"
device_id = 1
sensor_to_read = "ultrasound"
command = ""
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
s.write(packet_to_vehicle)

byte = s.read()
start_byte_byte = START_BYTE.to_bytes(1, byteorder='big')
if byte == start_byte_byte:
    print("legal")
    packet_from_vehicle = s.read_until(start_byte_byte, 50)
#packet_from_vehicle = s.readall()
print(type(packet_from_vehicle), len(packet_from_vehicle), packet_from_vehicle)


#interpret packet from vehicle

if(packet_from_vehicle and packet_from_vehicle[-1] == START_BYTE 
                                        and packet_from_vehicle[0] == device_id):
    print("pacote recebido: ")
    current_address = 1
    if sensor_to_read == "ultrasound":
        sensor_read = struct.unpack('f', packet_from_vehicle[current_address:(current_address+4)])
        current_address += 4
    elif sensor_to_read != "ultrasound":
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

s.close()
print("fim")
