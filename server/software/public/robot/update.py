import json
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
        public_key = robot.public_key
        if public_key:
            if objects.Robot.verify(public_key, data):
                if "state" in data:
                    robot.state = data["state"]
                if "item" in data:
                    robot.item = data["item"]
                if "qr" in data:
                    pass
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
