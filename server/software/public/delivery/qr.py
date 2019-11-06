import json
import asyncio
import objects

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "cookie" in data and "id" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        login = objects.Login(data["cookie"])
        email = login.get_email()
        if email != None:
            code = objects.QRCode.generate(email, data["id"])
            if code:
                print(code.number)
                resp["message_body"] = "true"
                await websocket.send(json.dumps(resp))
            else:
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
