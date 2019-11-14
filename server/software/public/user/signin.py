import json
import asyncio
import objects
import websockets

async def main(websocket, path, open_sockets, script_cache):
    module = objects.Module(__name__)
    login = None
    try:
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
                            if path == "/debug/close":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/debug/open":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/delivery/finish":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/delivery/qr":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/delivery/request":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/delivery/response":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/delivery/send":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/user/delete":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/user/profile":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/user/signout":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
                                except Exception as e:
                                    module.error(e, script.__name__)
                                    await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                            elif path == "/user/update":
                                script = script_cache.import_(path)
                                try:
                                    await script.main(websocket, path, open_sockets, script_cache, data)
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
        if login:
            open_sockets["users"].pop(login.cookie)
            login.delete()
