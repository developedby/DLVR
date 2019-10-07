import requests
import serial
import struct
import time

START_BYTE = 0Xaa
url_request = ""
url_post = ""

s = serial.Serial('/dev/ttyUSB0', 115200, timeout=5)
time.sleep(2)

#server
#variables from server
route = []
qr_code_destination = ""
required_status = ""
device_id = 0
sensor_to_read = ""
command = ""
#variables to server
qr_codes_read = []
sensor_reading = 0
status = ""
#dictionaries to server
movement = {"amplitude" : 0.00, "curvature" : 0.00}

#vehicle
#dictionaries to vehicle
vehicle_sensors_code = {"item detector" : 1, "ultrasound" : 2, "lock" : 3}
possible_commands = {"force go ahead" : 1, "open_box" : 2, "close_box" : 3}
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
    packet_to_vehicle.append(route)
    packet_to_vehicle.append(qr_code_destination)
if command:
    packet_to_vehicle.append(command)
if sensor_to_read:
    packet_to_vehicle.append(vehicle_sensors_code[sensor_to_read])
if required_status:
    packet_to_vehicle.append(possible_status_to_vehicle[required_status])
packet_to_vehicle.append(START_BYTE)
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

if(packet_from_vehicle and packet_from_vehicle[0] == START_BYTE and packet_from_vehicle[len(packet_from_vehicle)] == START_BYTE 
                                        and packet_from_vehicle[1] == device_id):
    current_address = 2
    if sensor_to_read == "ultrasound":
        sensor_read = struct.pack('f', packet_from_vehicle[current_address:(current_address+3)])
        current_address += 4
    elif sensor_to_read != "ultrasound":
            sensor_read = packet_from_vehicle[current_address]
            current_address += 1
    if required_status == "movement":
        movement["amplitude"] = struct.pack('f', packet_from_vehicle[current_address:(current_address+3)])
        current_address += 4
        movement["curvature"] = struct.pack('f',packet_from_vehicle[current_address:(current_address+3)])
        current_address += 4
        status = movement
    elif required_status == "status_robot":
        status = possible_status_from_vehicle[packet_from_vehicle[current_address]]
    qr_codes_read.append(packet_from_vehicle[current_address : len(packet_from_vehicle - 2)])

s.close()
print("fim")