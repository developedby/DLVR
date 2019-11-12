import json
import asyncio
import objects

async def main(id, open_sockets):
    module = objects.Module(__name__)
    try:
        resp = {
            "status_code": 200,
            "reason_message": "OK",
            "path": "/debug/close",
            "message_body": "true"
        }
        await open_sockets["robots"][id].send(json.dumps(resp))
    except Exception as e:
        module.error(e)
