#!/usr/bin/env python3

import asyncio
import websockets
import ssl
import json
import threading

CERT_PEM_PATH = './cert.pem'
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
ssl_context.load_verify_locations(CERT_PEM_PATH)
loop = asyncio.get_event_loop()

def main():
    loop.run_until_complete(handler())

async def handler():
    uri = "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
    data = {"email": "subject@example.com", "password": "Password123!", "first_name": "Subject", "last_name": "Subject"}
    async with websockets.connect(uri + "/user/check", ssl = ssl_context) as websocket:
        await websocket.send(json.dumps(data))
        resp = await websocket.recv()
        resp = json.loads(resp)
        if resp["status_code"] == 200 and resp["message_body"] == "true":
            print("Signup efetuado com sucesso")
            code = int(await loop.run_in_executor(None, input))
            data = {"user": "subject@example.com", "number": code}
            async with websockets.connect(uri + "/code/verify", ssl = ssl_context) as websocket2:
                await websocket2.send(json.dumps(data))
                resp = await websocket2.recv()
                resp = json.loads(resp)
                if resp["status_code"] == 200 and resp["message_body"] == "true":
                    print("Código verificado com sucesso")
                    data = {"email": "subject@example.com", "password": "Password123!"}
                    async with websockets.connect(uri + "/user/signin", ssl = ssl_context) as websocket3:
                        await websocket3.send(json.dumps(data))
                        resp = await websocket3.recv()
                        resp = json.loads(resp)
                        if resp["status_code"] == 200 and resp["message_body"] == "true":
                            print("Signin efetuado com sucesso")
                            cookie = resp["set_cookie"]
                            print("Cookie: {}".format(cookie))
                            async for message in websocket3:
                                print(message)
                                resp = json.loads(message)
                                if "path" in resp:
                                    if resp["path"] == "/delivery/request":
                                        await loop.run_in_executor(None, input)
                                        data = {"path": "/delivery/response", "cookie": cookie, "id": resp["message_body"]["id"], "accept": True, "destination": 0}
                                        await websocket3.send(json.dumps(data))
                                        data = {"path": "/user/delete", "cookie": cookie, "password": "Password123!"}
                                        await websocket3.send(json.dumps(data))
                                        await loop.run_in_executor(None, input)
                                        return
                        else:
                            print(resp)
                            print("Signin incorreto")
                else:
                    print(resp)
                    print("Código incorreto")
        else:
            print(resp)
            print("Signup incorreto")

if __name__ == "__main__":
    main()
