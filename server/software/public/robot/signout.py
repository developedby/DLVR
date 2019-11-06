import json
import datetime
import hashlib
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
        public_key = robot.get_public_key()
        if public_key:
            data_no_sign = data.copy()
            data_no_sign.pop("signature")
            data_no_sign = json.dumps(data_no_sign, sort_keys = True).encode("utf-8")
            hash = hashlib.sha256(data_no_sign).hexdigest().encode("utf-8")
            if public_key.verify(hash, (data["signature"],)):
                timestamp = datetime.datetime.fromtimestamp(data["timestamp"])
                now = datetime.datetime.now()
                diff = min((now - timestamp).seconds, (timestamp - now).seconds)
                if diff < 60:
                    if robot.signout():
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
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
