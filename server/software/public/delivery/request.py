import json
import asyncio
import objects

async def main(websocket, path, open_sockets, script_cache, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "origin" in data and "receiver" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        user = objects.Login(data["cookie"]).user
        if user:
            logins = objects.User(data["receiver"]).logins
            if len(logins) > 0:
                delivery = objects.Delivery.create(data["origin"], user.email, data["receiver"])
                if delivery:
                    for login in logins:
                        data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/request", "message_body": {"id": delivery.id, "sender": user.email, "origin": data["origin"]}}
                        await open_sockets["users"][login.cookie].send(json.dumps(data2))
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
