import json
import asyncio
import objects

async def main(websocket, path, open_sockets, data = None):
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
                            robot = objects.Robot.choose(origin)
                            if robot:
                                if delivery.response(data["destination"], robot.id):
                                    for login in logins:
                                        data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"id": delivery.id, "accept": True, "destination": data["destination"]}}
                                        await open_sockets["users"][login.cookie].send(json.dumps(data2))
                                    robot_path = ""
                                    #choose_robot.choose_path(origin)
                                    data3 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"path": robot_path}}
                                    await open_sockets["robots"][robot.id].send(json.dumps(data3))
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
