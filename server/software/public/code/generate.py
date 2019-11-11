import json
import asyncio
import objects

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "user" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        code = objects.Code.create(data["user"])
        if code:
            user = code.user
            objects.Request(websocket).log(path, code.number)
            resp["message_body"] = "true"
            await websocket.send(json.dumps(resp))
            objects.send_email(user.email, user.first_name, user.last_name, code.number)
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
