import json
import asyncio
import objects

async def main(websocket, path, open_sockets, script_cache, data = None, id = 0):
    module = objects.Module(__name__)
    if data == None:
        data = await websocket.recv()
        data = json.loads(data)
    resp = {
        "status_code": 200,
        "reason_message": "OK",
        "path": "/debug/open",
        "message_body": "true"
    }
    try:
        await open_sockets["robots"][id].send(json.dumps(resp))
        if websocket:
            await websocket.send(json.dumps(resp))
    except Exception as e:
        module.error(e)
