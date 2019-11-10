import json
import asyncio
import objects

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "password" in data and ("email" in data or "first_name" in data or "last_name" in data or "new_password" in data):
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        login = objects.Login(data["cookie"])
        user = login.user
        if user:
            if user.password == data["password"]:
                if "email" in data:
                    user.email = data["email"]
                if "first_name" in data:
                    user.first_name = data["first_name"]
                if "last_name" in data:
                    user.last_name = data["last_name"]
                if "new_password" in data:
                    user.password = data["new_password"]
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
