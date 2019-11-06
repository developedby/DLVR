import json
import asyncio
import hashlib
import datetime
import objects

async def main(websocket, path, open_sockets):
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
            if objects.Robot.verify(public_key, data):
                if robot.signin():
                    resp["message_body"] = "true"
                    await websocket.send(json.dumps(resp))
                    open_sockets["robots"][robot.id] = websocket
                    async for message in websocket:
                        data = json.loads(message)
                        if "path" in data:
                            path = data["path"]
                            if path == "/robot/route":
                                import public.robot.route as script
                                try:
                                    await script.main(websocket, path, open_sockets, data)
                                except Exception as e:
                                    print("signin.py:52(" + script.__name__ + "): " + str(e))
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/robot/signout":
                                import public.robot.signout as script
                                try:
                                    await script.main(websocket, path, open_sockets, data)
                                except Exception as e:
                                    print("signin.py:59(" + script.__name__ + "): " + str(e))
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/robot/update":
                                import public.robot.update as script
                                try:
                                    await script.main(websocket, path, open_sockets, data)
                                except Exception as e:
                                    print("signin.py:66(" + script.__name__ + "): " + str(e))
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            else:
                                await websocket.send("{\"status_code\": 404, \"reason_message\": \"Not Found\"}")
                        else:
                            await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
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
