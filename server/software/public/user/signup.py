import json
import connect
import mysql.connector
import os
import hashlib
import asyncio
import websockets
import ssl

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data and "first_name" in data and "last_name" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        connection = connect.connect()
        cursor = connection.cursor(prepared = True)
        query = "INSERT INTO User(email, verified, first_name, last_name, salt, hash) VALUES (%s, %s, %s, %s, %s, %s)"
        salt = os.urandom(32)
        hash = hashlib.pbkdf2_hmac("sha256", data["password"].encode("utf-8"), salt, 100000)
        values = (data["email"], 0, data["first_name"], data["last_name"], salt.hex(), hash.hex(),)
        try:
            cursor.execute(query, values)
            connection.commit()
            CERT_PEM_PATH = '../cert.pem'
            ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
            ssl_context.load_verify_locations(CERT_PEM_PATH)
            uri = "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
            data2 = {"email": data["email"]}
            async with websockets.connect(uri + "/code/generate", ssl = ssl_context) as websocket2:
                await websocket2.send(json.dumps(data2))
                resp2 = await websocket2.recv()
                await websocket.send(resp2)
        except Exception as e:
            print("signup.py:37: " + str(e))
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
        cursor.close()
        connection.close()
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
