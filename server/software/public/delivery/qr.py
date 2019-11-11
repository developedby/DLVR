import json
import asyncio
import objects

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "cookie" in data and "id" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        login = objects.Login(data["cookie"])
        user = login.user
        if user:
            code = objects.QRCode.create(data["id"])
            if code:
                objects.Request(websocket).log(path, code.number)
                delivery = code.delivery
                delivery.qr()
                robot = delivery.robot
                data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/qr", "message_body": code.number}
                await open_sockets["robots"][robot.id].send(json.dumps(data2))
                resp["message_body"] = code.number
                await websocket.send(json.dumps(resp))
            else:
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
