import json
import connect
import mysql.connector
import asyncio

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        connection = connect.connect()
        cursor = connection.cursor(prepared = True)
        query = "SELECT email FROM User WHERE email = %s"
        values = (data["email"],)
        try:
            cursor.execute(query, values)
            result = cursor.fetchone()
            resp["message_body"] = "true" if result else "false"
            await websocket.send(json.dumps(resp))
        except mysql.connector.Error as e:
            print("check.py:24: " + str(e))
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
        cursor.close()
        connection.close()
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
