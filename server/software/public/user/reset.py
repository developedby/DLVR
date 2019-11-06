import json
import asyncio
import objects

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data and "code" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        if objects.Code(data["code"]).verify(data["email"]) and objects.User(data["email"]).set_password(data["password"]):
            resp["message_body"] = "true"
            await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
