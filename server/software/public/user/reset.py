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
    if "email" in data and "code" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        CERT_PEM_PATH = '../cert.pem'
        ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        ssl_context.load_verify_locations(CERT_PEM_PATH)
        uri = "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
        async with websockets.connect(uri + "/code/verify", ssl = ssl_context) as websocket2:
            data2 = {"email": data["email"], "number": data["code"]}
            await websocket2.send(json.dumps(data2))
            resp2 = await websocket2.recv()
            resp2 = json.loads(resp2)
            if resp2["message_body"] == "true":
                connection = connect.connect()
                cursor = connection.cursor(prepared = True)
                query = "UPDATE User SET salt = %s, hash = %s WHERE email = %s"
                salt = os.urandom(32)
                hash = hashlib.pbkdf2_hmac("sha256", data["password"].encode("utf-8"), salt, 100000)
                values = (salt.hex(), hash.hex(), data["email"],)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    resp["message_body"] = "true"
                    await websocket.send(json.dumps(resp))
                except mysql.connector.Error as e:
                    print("reset.py:40: " + str(e))
                    connection.rollback()
                    resp["message_body"] = "false"
                    await websocket.send(json.dumps(resp))
                cursor.close()
                connection.close()
            else:
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
