import json
import asyncio

async def main(websocket, path):
    resp = {
        "status_code": 200,
        "reason_message": "OK"
    }
    await websocket.send(json.dumps(resp))
