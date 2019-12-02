#!/usr/bin/env python3

import asyncio
import websockets
import ssl
import json

CERT_PEM_PATH = './cert.pem'
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
ssl_context.load_verify_locations(CERT_PEM_PATH)
loop = asyncio.get_event_loop()

def main():
    loop.run_until_complete(handler())

async def handler():
    uri = "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
    print("Options:")
    print("0 - Signs in and signs out")
    print("1 - Signs up and signs out")
    print("2 - Signs up and deletes account")
    print("Enter one option: ", end = "")
    option = int(await loop.run_in_executor(None, input))
    if option > 0:
        data = {"email": "subject2@example.com", "first_name": "Subject2", "last_name": "Subject2", "password": "Password456!"}
        async with websockets.connect(uri + "/user/signup", ssl = ssl_context) as websocket:
            await websocket.send(json.dumps(data))
            resp = await websocket.recv()
            resp = json.loads(resp)
            if resp["status_code"] == 200 and resp["message_body"] == "true":
                print("Signup done successfully")
                print("Enter the verification code: ", end = "")
                number = int(await loop.run_in_executor(None, input))
                data = {"user": "subject2@example.com", "number": number}
                async with websockets.connect(uri + "/code/verify", ssl = ssl_context) as websocket2:
                    await websocket2.send(json.dumps(data))
                    resp = await websocket2.recv()
                    resp = json.loads(resp)
                    if resp["status_code"] == 200 and resp["message_body"] == "true":
                        print("Code verified successfully")
                    else:
                        print(resp)
                        print("Incorrect code")
            else:
                print(resp)
                print("Incorrect signup")
    data = {"email": "subject2@example.com", "password": "Password456!"}
    async with websockets.connect(uri + "/user/signin", ssl = ssl_context) as websocket:
        await websocket.send(json.dumps(data))
        resp = await websocket.recv()
        resp = json.loads(resp)
        if resp["status_code"] == 200 and resp["message_body"] == "true":
            print("Signin done successfully")
            cookie = resp["set_cookie"]
            print("Cookie: {}".format(cookie))
            ORIG = 15
            id = None
            print("Press enter to continue...", end = "")
            await loop.run_in_executor(None, input)
            data = {"path": "/delivery/request", "cookie": cookie, "origin": ORIG, "receiver": "subject@example.com"}
            await websocket.send(json.dumps(data))
            resp = await websocket.recv()
            resp = json.loads(resp)
            if resp["status_code"] == 200 and resp["message_body"] == "true":
                print("Delivery request sent")
                async for message in websocket:
                    print(message)
                    resp = json.loads(message)
                    if "path" in resp:
                        if resp["path"] == "/delivery/response":
                            id = resp["message_body"]["id"]
                            print("Delivery accepted")
                        elif resp["path"] == "/robot/update":
                            if "position" in resp["message_body"] and "state" in resp["message_body"] and resp["message_body"]["state"] == 1 and resp["message_body"]["position"] == ORIG:
                                print("Robot arrived at origin")
                                print("Press enter to continue...", end = "")
                                await loop.run_in_executor(None, input)
                                data = {"path": "/delivery/send", "cookie": cookie, "id": id}
                                await websocket.send(json.dumps(data))
                        elif resp["path"] == "/delivery/finish":
                            print("Delivery finished")
                            if option == 2:
                                data = {"path": "/user/delete", "cookie": cookie, "password": "Password456!"}
                            else:
                                data = {"path": "/user/signout", "cookie": cookie}
                            await websocket.send(json.dumps(data))
                            print("Press enter to continue...", end = "")
                            await loop.run_in_executor(None, input)
                            return
            else:
                print(json.dumps(data))
                print(resp)
                print("Incorrect delivery request")
        else:
            print(resp)
            print("Incorrect signin")

if __name__ == "__main__":
    main()
