import json
import asyncio
import objects
import websockets

async def main(websocket, path, open_sockets):
    module = objects.Module(__name__)
    robot = None
    try:
        req = objects.Request(websocket)
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
                    robot.alive = True
                    resp["message_body"] = "true"
                    await websocket.send(json.dumps(resp))
                    open_sockets["robots"][robot.id] = websocket
                    async for message in websocket:
                        data = json.loads(message)
                        if "path" in data:
                            path = data["path"]
                            req.log(path)
                            if path == "/robot/route":
                                import public.robot.route as script
                                try:
                                    await script.main(websocket, path, open_sockets, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/robot/signout":
                                import public.robot.signout as script
                                try:
                                    await script.main(websocket, path, open_sockets, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/robot/update":
                                import public.robot.update as script
                                try:
                                    await script.main(websocket, path, open_sockets, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
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
            await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
    except websockets.exceptions.ConnectionClosedError as e:
        module.error(e)
        if robot:
            open_sockets["robots"].pop(robot.id)
            robot.alive = False
