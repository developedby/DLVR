import json
import asyncio

async def main(websocket, path, data = None):
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
            resp["message_body"] = "true"
            await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
