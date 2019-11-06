import json
import hashlib
import asyncio
import objects

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "id" in data and "signature" in data and "timestamp" in data and ("state" in data or "item" in data or "qr" in data or "speed" in data or "curve_radius" in data or "left_encoder" in data or "right_encoder" in data or "ultrasound" in data):
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        robot = objects.Robot(data["id"])
        public_key = robot.get_public_key()
        if public_key:
            if objects.Robot.verify(public_key, data):
                errors = 0

                if "state" in data:
                    if not robot.set_state(data["state"]):
                        errors += 1

                if "item" in data:
                    if not robot.set_item(data["item"]):
                        errors += 1

                if "qr" in data:
                    pass

                if "speed" in data:
                    if not robot.set_speed(data["speed"]):
                        errors += 1

                if "curve_radius" in data:
                    if not robot.set_curve_radius(data["curve_radius"]):
                        errors += 1

                if "left_encoder" in data:
                    if not robot.set_left_encoder(data["left_encoder"]):
                        errors += 1

                if "right_encoder" in data:
                    if not robot.set_right_encoder(data["right_encoder"]):
                        errors += 1

                if "ultrasound" in data:
                    if not robot.set_ultrasound(data["ultrasound"]):
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
