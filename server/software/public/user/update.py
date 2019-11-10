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
        user = login.get_user()
        if user:
            if user.check_password(data["password"]):
                errors = 0
                if "email" in data:
                    if not user.set_email(data["email"]):
                        errors += 1
                if "first_name" in data:
                    if not user.set_first_name(data["first_name"]):
                        errors += 1
                if "last_name" in data:
                    if not user.set_last_name(data["last_name"]):
                        errors += 1
                if "new_password" in data:
                    if not user.set_password(data["new_password"]):
                        errors += 1
                if errors == 0:
                    resp["message_body"] = "true"
                    await websocket.send(json.dumps(resp))
                else:
                    resp["message_body"] = "false"
                    await websocket.send(json.dumps(resp))
            else:
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
