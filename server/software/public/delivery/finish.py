import json
import asyncio
import objects

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "id" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        user = objects.Login(data["cookie"]).user
        if user:
            delivery = objects.Delivery(data["id"])
            receiver = delivery.receiver
            if receiver:
                logins = receiver.logins
                if len(logins) > 0:
                    robot = delivery.robot
                    delivery.finish()
                    data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/finish", "message_body": "true"}
                    data2 = json.dumps(data2)
                    for login in logins:
                        await open_sockets["users"][login.cookie].send(data2)
                    await open_sockets["robots"][robot.id].send(data2)
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
