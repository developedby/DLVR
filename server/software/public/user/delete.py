import json
import asyncio
import objects

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        login = objects.Login(data["cookie"])
        email = login.get_email()
        if email != None:
            user = objects.User(email)
            if user.check_password(data["password"]) and login.signout() and user.delete():
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
