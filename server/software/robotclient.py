#!/usr/bin/env python3

import json
import datetime
import hashlib
import Crypto.PublicKey.RSA
import asyncio
import websockets
import ssl

CERT_PEM_PATH = './cert.pem'
ROBOT_DER_PATH = './robot0.der'
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
ssl_context.load_verify_locations(CERT_PEM_PATH)
loop = asyncio.get_event_loop()

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
            data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "qr" : 23, "state": 6}
            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
            await websocket.send(json.dumps(data))
            async for message in websocket:
                print(message)
                resp = json.loads(message)
                if "path" in resp:
                    if resp["path"] == "/delivery/response":
                        print("Starting delivery")
                        print("Press enter to continue...", end = "")
                        await loop.run_in_executor(None, input)
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 2}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        for direction, _, position in resp["message_body"]["path"]:
                            data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "qr": position, "direction": direction}
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 1, "qr": resp["message_body"]["path"][-1][2]}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        print("Container opened")
                    elif resp["path"] == "/delivery/send":
                        print("Container closed")
                        print("Press enter to continue...", end = "")
                        await loop.run_in_executor(None, input)
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 2}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        for direction, _, position in resp["message_body"]["path"]:
                            data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "qr": position, "direction": direction}
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 1, "qr": resp["message_body"]["path"][-1][2]}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        print("Container opened")
                    elif resp["path"] == "/delivery/qr":
                        print("QR code read")
                        print("Press enter to continue...", end = "")
                        await loop.run_in_executor(None, input)
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "qr": resp["message_body"]}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                    elif resp["path"] == "/delivery/finish":
                        print("Delivery finished")
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 2}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        for direction, _, position in resp["message_body"]["path"]:
                            data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "qr": position, "direction": direction}
                            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                            await websocket.send(json.dumps(data))
                        data = {"path": "/robot/update", "id": 0, "timestamp": datetime.datetime.now().timestamp(), "state": 1, "qr": resp["message_body"]["path"][-1][2]}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        data = {"path": "/robot/signout", "id": 0, "timestamp": datetime.datetime.now().timestamp()}
                        data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
                        await websocket.send(json.dumps(data))
                        print("Press enter to continue...", end = "")
                        await loop.run_in_executor(None, input)
                        return
        else:
            print(resp)
            print("Incorrect signin")

if __name__ == "__main__":
    main()
