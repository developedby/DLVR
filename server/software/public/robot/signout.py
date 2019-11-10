import json
import asyncio
import objects

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "id" in data and "signature" in data and "timestamp" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        robot = objects.Robot(data["id"])
        public_key = robot.public_key
        if public_key:
            if objects.Robot.verify(public_key, data):
                robot.alive = False
                resp["message_body"] = "true"
                await websocket.send(json.dumps(resp))
                open_sockets["robots"].pop(data["id"])
            else:
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
