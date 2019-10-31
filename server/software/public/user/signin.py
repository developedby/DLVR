import json
import session
import asyncio

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        cookie = session.signin(data["email"], data["password"])
        if cookie:
            resp["set_cookie"] = cookie.output(header = "", sep = "")
            resp["message_body"] = "true"
            await websocket.send(json.dumps(resp))
            open_sockets["users"][cookie["token"].value] = websocket
            async for message in websocket:
                data = json.loads(message)
                if "path" in data:
                    path = data["path"]
                    if path == "/delivery/finish":
                        import public.delivery.finish as script
                        await script.main(websocket, path, open_sockets, data)
                    elif path == "/delivery/request":
                        import public.delivery.request as script
                        await script.main(websocket, path, open_sockets, data)
                    elif path == "/delivery/response":
                        import public.delivery.response as script
                        await script.main(websocket, path, open_sockets, data)
                    elif path == "/delivery/send":
                        import public.delivery.send as script
                        await script.main(websocket, path, open_sockets, data)
                    elif path == "/user/delete":
                        import public.user.delete as script
                        await script.main(websocket, path, open_sockets, data)
                    elif path == "/user/signout":
                        import public.user.signout as script
                        await script.main(websocket, path, open_sockets, data)
                    elif path == "/user/update":
                        import public.user.update as script
                        await script.main(websocket, path, open_sockets, data)
                    else:
                        await websocket.send("{\"status_code\": 404, \"reason_message\": \"Not Found\"}")
                else:
                    await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
