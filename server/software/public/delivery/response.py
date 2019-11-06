import json
import asyncio
import objects
import private.choose_robot

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "id" in data and "accept" in data and (not data["accept"] or "destination" in data):
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        email = objects.Login(data["cookie"]).get_email()
        if email != None:
            delivery = objects.Delivery(data["id"])
            sender = delivery.get_sender()
            if sender != None:
                cookies = objects.Login.get_cookies(sender)
                if len(cookies) > 0:
                    if data["accept"]:
                        origin = delivery.get_origin()
                        if origin != None:
                            rid = private.choose_robot.choose(origin)
                            if delivery.response(data["destination"], rid):
                                for cookie in cookies:
                                    data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"id": delivery.id, "accept": True, "destination": data["destination"]}}
                                    await open_sockets["users"][cookie[0]].send(json.dumps(data2))
                                robot_path = ""
                                #choose_robot.choose_path(origin)
                                data3 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"path": robot_path}}
                                await open_sockets["robots"][rid].send(json.dumps(data3))
                                resp["message_body"] = "true"
                                await websocket.send(json.dumps(resp))
                            else:
                                resp["message_body"] = "false"
                                await websocket.send(json.dumps(resp))
                        else:
                            resp["message_body"] = "false"
                            await websocket.send(json.dumps(resp))
                    else:
                        for cookie in cookies:
                            data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"id": delivery.id, "accept": False}}
                            await open_sockets["users"][cookie[0]].send(json.dumps(data2))
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
