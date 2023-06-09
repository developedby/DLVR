import json
import asyncio
import objects

async def main(websocket, path, open_sockets, script_cache, data = None):
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
            sender = delivery.sender
            if sender:
                logins = sender.logins
                if len(logins) > 0:
                    robot = delivery.robot
                    delivery.finish()
                    robot_path = None
                    for garage in objects.garages:
                        route = objects.shortest_path(objects.city, delivery.destination, garage)
                        if robot_path == None or len(route) < len(robot_path):
                            robot_path = route
                    robot.route = robot_path
                    robot.orientation = -objects.city[robot.position][robot_path[1]][0]
                    robot_path = objects.path_to_directions(objects.city, robot_path, robot.orientation, robot.TURN_DISCOUNT)
                    data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/finish", "message_body": {"path": robot_path}}
                    await open_sockets["robots"][robot.id].send(json.dumps(data2))
                    data2["message_body"] = "true"
                    data2 = json.dumps(data2)
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
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
