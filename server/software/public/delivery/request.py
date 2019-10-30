import json
import session
import connect
import mysql.connector
import http.cookies
import asyncio

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "origin" in data and "email" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        cookie = http.cookies.SimpleCookie()
        cookie.load(data["cookie"])
        email = session.user_email(cookie["token"].value)
        if email:
            CERT_PEM_PATH = '../cert.pem'
            ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
            ssl_context.load_verify_locations(CERT_PEM_PATH)
            uri = "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
            data2 = {"email": data["email"]}
            async with websockets.connect(uri + "/user/check", ssl = ssl_context) as websocket2:
                await websocket2.send(json.dumps(data2))
                resp2 = await websocket2.recv()
                resp2 = json.loads(resp2)
                if resp2["message_body"] == "true":
                    resp["message_body"] = "true"
                    await websocket.send(json.dumps(resp))
                else:
                    resp["message_body"] = "false"
                    await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
