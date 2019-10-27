import json
import connect
import mysql.connector
import os
import datetime
import random
import asyncio

async def main(websocket, path):
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data and "number" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        connection = connect.connect()
        cursor = connection.cursor(prepared = True)
        query = "DELETE FROM Code WHERE expiration < %s"
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        values = (timestamp,)
        try:
            cursor.execute(query, values)
            connection.commit()
            query = "SELECT user FROM Code WHERE number = %s"
            values = (data["number"],)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result and result[0] == data["email"]:
                    query = "DELETE FROM Code WHERE number = %s"
                    values = (data["number"],)
                    try:
                        cursor.execute(query, values)
                        query = "UPDATE User SET verified = 1 WHERE email = %s"
                        values = (data["email"],)
                        try:
                            cursor.execute(query, values)
                            connection.commit()
                            resp["message_body"] = "true"
                            await websocket.send(json.dumps(resp))
                        except mysql.connector.Error as e:
                            print("verify.py:43: " + str(e))
                            connection.rollback()
                            resp["message_body"] = "false"
                            await websocket.send(json.dumps(resp))
                    except mysql.connector.Error as e:
                        print("verify.py:48: " + str(e))
                        connection.rollback()
                        resp["message_body"] = "false"
                        await websocket.send(json.dumps(resp))
                else:
                    resp["message_body"] = "false"
                    await websocket.send(json.dumps(resp))
            except mysql.connector.Error as e:
                print("verify.py:56: " + str(e))
                connection.rollback()
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
        except mysql.connector.Error as e:
            print("verify.py:61: " + str(e))
            connection.rollback()
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
        cursor.close()
        connection.close()
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
