import json
import asyncio
import objects

async def main(websocket, path, open_sockets, script_cache):
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data and "code" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        code = objects.Code(data["code"])
        user = code.user
        if user and user.email == data["email"]:
            user.password = data["password"]
            code.delete()
            resp["message_body"] = "true"
            await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
