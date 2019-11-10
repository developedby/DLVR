import json
import asyncio
import objects

async def main(websocket, path, open_sockets, data = None):
    data = await websocket.recv()
    data = json.loads(data)
    if "cookie" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        user = objects.Login(data["cookie"]).user
        if user:
            resp["message_body"] = {}
            first_name = user.first_name
            if first_name != None:
                resp["message_body"]["first_name"] = first_name
            last_name = user.last_name
            if last_name != None:
                resp["message_body"]["last_name"] = last_name
            await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
