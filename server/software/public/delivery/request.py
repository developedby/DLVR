import json
import asyncio
import objects

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "origin" in data and "email" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        email = objects.Login(data["cookie"]).get_email()
        if email != None:
            cookies = objects.Login.get_cookies(data["email"])
            if len(cookies) > 0:
                delivery = objects.Delivery.request(email, data["origin"], data["email"])
                if delivery:
                    for cookie in cookies:
                        data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/request", "message_body": {"id": delivery.id, "sender": email, "origin": data["origin"]}}
                        await open_sockets["users"][cookie[0]].send(json.dumps(data2))
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
