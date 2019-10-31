import json
import connect
import mysql.connector
import os
import datetime
import random
import asyncio

async def main(websocket, path, open_sockets):
    data = await websocket.recv()
    data = json.loads(data)
    if "email" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM Code WHERE expiration < %s"
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (timestamp,)
            try:
                cursor.execute(query, values)
                query = "DELETE FROM Code WHERE user = %s"
                values = (data["email"],)
                try:
                    cursor.execute(query, values)
                    query = "INSERT INTO Code(number, expiration, user) VALUES (%s, %s, %s)"
                    number = random.randint(0, 65535)
                    expiration = datetime.datetime.now() + datetime.timedelta(minutes = 5)
                    timestamp = expiration.strftime("%Y-%m-%d %H:%M:%S")
                    values = (number, timestamp, data["email"],)
                    try:
                        cursor.execute(query, values)
                        connection.commit()
                        print(number)
                        resp["message_body"] = "true"
                        await websocket.send(json.dumps(resp))
                    except mysql.connector.Error as e:
                        print("generate.py:40: " + str(e))
                        connection.rollback()
                        resp["message_body"] = "false"
                        await websocket.send(json.dumps(resp))
                except mysql.connector.Error as e:
                    print("generate.py:45: " + str(e))
                    connection.rollback()
                    resp["message_body"] = "false"
                    await websocket.send(json.dumps(resp))
            except mysql.connector.Error as e:
                print("generate.py:50: " + str(e))
                connection.rollback()
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
            cursor.close()
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
