import json
import asyncio
import objects

async def main(websocket, path, open_sockets, script_cache, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "id" in data and "accept" in data and (not data["accept"] or "destination" in data):
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        user = objects.Login(data["cookie"]).user
        if user:
            delivery = objects.Delivery(data["id"])
            sender = delivery.sender
            if sender:
                logins = sender.logins
                if len(logins) > 0:
                    if data["accept"]:
                        origin = delivery.origin
                        if origin != None:
                            nearest = objects.Robot.choose(origin)
                            if nearest:
                                robot = nearest[0]
                                robot_path = nearest[1]
                                robot.route = robot_path
                                robot.orientation = -objects.city[robot.position][robot_path[1]][0]
                                robot_path = objects.path_to_directions(objects.city, robot_path, robot.orientation, robot.TURN_DISCOUNT)
                                if delivery.response(robot.position, data["destination"], robot.id):
                                    for login in logins:
                                        data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"id": delivery.id, "accept": True, "destination": data["destination"], "path": robot_path}}
                                        await open_sockets["users"][login.cookie].send(json.dumps(data2))
                                    data3 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"path": robot_path}}
                                    await open_sockets["robots"][robot.id].send(json.dumps(data3))
                                    resp["message_body"] = {"path": robot_path}
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
                        for login in logins:
                            data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"id": delivery.id, "accept": False}}
                            await open_sockets["users"][login.cookie].send(json.dumps(data2))
                        if delivery.delete():
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
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
