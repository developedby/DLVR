import json
import asyncio
import objects

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        user = objects.User(data["email"])
        if user.get_verified() and user.check_password(data["password"]):
            login = objects.Login.signin(data["email"])
            if login:
                resp["set_cookie"] = login.cookie
                resp["message_body"] = "true"
                await websocket.send(json.dumps(resp))
                open_sockets["users"][login.cookie] = websocket
                async for message in websocket:
                    data = json.loads(message)
                    if "path" in data:
                        path = data["path"]
                        if path == "/delivery/finish":
                            import public.delivery.finish as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                print("signin.py:28(" + script.__name__ + "): " + str(e))
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/delivery/qr":
                            import public.delivery.qr as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                print("signin.py:35(" + script.__name__ + "): " + str(e))
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/delivery/request":
                            import public.delivery.request as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                print("signin.py:42(" + script.__name__ + "): " + str(e))
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/delivery/response":
                            import public.delivery.response as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                print("signin.py:49(" + script.__name__ + "): " + str(e))
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/delivery/send":
                            import public.delivery.send as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                print("signin.py:56(" + script.__name__ + "): " + str(e))
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/user/delete":
                            import public.user.delete as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                print("signin.py:63(" + script.__name__ + "): " + str(e))
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/user/signout":
                            import public.user.signout as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                print("signin.py:70(" + script.__name__ + "): " + str(e))
                                await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
                        elif path == "/user/update":
                            import public.user.update as script
                            try:
                                await script.main(websocket, path, open_sockets, data)
                            except Exception as e:
                                print("signin.py:77(" + script.__name__ + "): " + str(e))
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
