#!/usr/bin/env python3
import asyncio
import datetime
import random
import websockets
import ssl
import pathlib
import os

HOST = ""
PORT = 80
#CERT_PEM_PATH = '../cert.pem'
#KEY_PEM_PATH  = '../key.pem'

async def time(websocket, path):
    print(f"Connection with {websocket}, path: {path}")
    for _ in range(5):
        now = datetime.datetime.utcnow().isoformat() + "Z"
        await websocket.send(now)
        print(f"    Sending {now} to {websocket}")
        await asyncio.sleep(random.random() * 30)
    print("Connection closed!")

#ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
#ssl_context.load_cert_chain(CERT_PEM_PATH, KEY_PEM_PATH, None)

start_server = websockets.serve(
    time, HOST, PORT, #ssl=ssl_context
)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()

