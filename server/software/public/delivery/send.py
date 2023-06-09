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
            receiver = delivery.receiver
            if receiver:
                logins = receiver.logins
                if len(logins) > 0:
                    robot = delivery.robot
                    delivery.send()
                    robot_path = objects.shortest_path(objects.city, delivery.origin, delivery.destination)
                    robot.route = robot_path
                    robot.orientation = -objects.city[robot.position][robot_path[1]][0]
                    robot_path = objects.path_to_directions(objects.city, robot_path, robot.orientation, robot.TURN_DISCOUNT)
                    data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/send", "message_body": {"path": robot_path}}
                    data2 = json.dumps(data2)
                    for login in logins:
                        await open_sockets["users"][login.cookie].send(data2)
                    await open_sockets["robots"][robot.id].send(data2)
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
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
