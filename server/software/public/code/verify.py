import json
import asyncio
import objects

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "user" in data and "number" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        if objects.Code(data["number"]).verify(data["user"]):
            user = objects.User(data["user"])
            if user.get_verified() or user.verify():
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
