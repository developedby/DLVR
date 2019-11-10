import json
import asyncio
import objects

async def main(websocket, path, open_sockets):
    module = objects.Module(__name__)
    req = objects.Request(websocket)
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        user = objects.User(data["email"])
        if user.verified and user.password == data["password"]:
            login = objects.Login.create(data["email"])
            if login:
                resp["set_cookie"] = login.cookie
                resp["message_body"] = "true"
                await websocket.send(json.dumps(resp))
                open_sockets["users"][login.cookie] = websocket
                async for message in websocket:
                    data = json.loads(message)
                    if "path" in data:
                        path = data["path"]
                        req.log(path)
                        if path == "/delivery/finish":
                            import public.delivery.finish as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                module.error(e, script.__name__)
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/delivery/qr":
                            import public.delivery.qr as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                module.error(e, script.__name__)
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/delivery/request":
                            import public.delivery.request as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                module.error(e, script.__name__)
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/delivery/response":
                            import public.delivery.response as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                module.error(e, script.__name__)
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/delivery/send":
                            import public.delivery.send as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                module.error(e, script.__name__)
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/user/delete":
                            import public.user.delete as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                module.error(e, script.__name__)
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/user/profile":
                            import public.user.profile as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                module.error(e, script.__name__)
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/user/signout":
                            import public.user.signout as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                module.error(e, script.__name__)
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/user/update":
                            import public.user.update as script
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
