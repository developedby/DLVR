import json
import asyncio
import objects

async def main(websocket, path, open_sockets, script_cache, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        login = objects.Login(data["cookie"])
        open_sockets["users"].pop(login.cookie)
        login.delete()
        resp["message_body"] = "true"
        await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
