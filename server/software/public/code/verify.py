import json
import asyncio
import objects

async def main(websocket, path, open_sockets, script_cache):
    data = await websocket.recv()
    data = json.loads(data)
    if "user" in data and "number" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        code = objects.Code(data["number"])
        user = code.user
        if user and user.email == data["user"]:
            if not user.verified:
                user.verified = True
            code.delete()
            resp["message_body"] = "true"
            await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
