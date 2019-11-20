import json
import asyncio
import objects

async def main(websocket, path, open_sockets, script_cache, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "id" in data and "signature" in data and "timestamp" in data and ("state" in data or "item" in data or "qr" in data or "direction" in data or "speed" in data or "curve_radius" in data or "left_encoder" in data or "right_encoder" in data or "ultrasound" in data):
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        robot = objects.Robot(data["id"])
        public_key = robot.public_key
        if public_key:
            if objects.Robot.verify(public_key, data):
                if "state" in data:
                    robot.state = data["state"]
                if "item" in data:
                    robot.item = data["item"]
                if "qr" in data:
                    if data["qr"] < 32:
                        robot.position = data["qr"]
                if "direction" in data:
                    robot.orientation += objects.Direction.fromstr(data["direction"])
                if "speed" in data:
                    robot.speed = data["speed"]
                if "curve_radius" in data:
                    robot.curve_radius = data["curve_radius"]
                if "left_encoder" in data:
                    robot.left_encoder = data["left_encoder"]
                if "right_encoder" in data:
                    robot.right_encoder = data["right_encoder"]
                if "ultrasound" in data:
                    robot.ultrasound = data["ultrasound"]
                if robot.delivery != None:
                    delivery = robot.delivery
                    sender = delivery.sender
                    receiver = delivery.receiver
                    data2 = {"status_code": 200, "reason_message": "OK", "path": "/robot/update"}
                    data2["message_body"] = data.copy()
                    data2["message_body"].pop("id")
                    data2["message_body"].pop("signature")
                    data2["message_body"].pop("timestamp")
                    if "qr" in data2["message_body"] and data2["message_body"]["qr"] < 32:
                        if len(delivery.path) == 0 or delivery.path[-1] != data2["message_body"]["qr"]:
                            delivery.path = delivery.path + [data2["message_body"]["qr"]]
                        data2["message_body"]["position"] = data2["message_body"]["qr"]
                        data2["message_body"].pop("qr")
                    data2 = json.dumps(data2)
                    logins = sender.logins
                    for login in logins:
                        await open_sockets["users"][login.cookie].send(data2)
                    logins = receiver.logins
                    for login in logins:
                        await open_sockets["users"][login.cookie].send(data2)
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
