import json
import session
import connect
import mysql.connector
import datetime
import http.cookies
import asyncio

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "origin" in data and "email" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        cookie = http.cookies.SimpleCookie()
        cookie.load(data["cookie"])
        email = session.user_email(cookie["token"].value)
        if email:
            cookies = session.user_cookies(data["email"])
            if len(cookies) > 0:
                with connect.connect() as connection:
                    cursor = connection.cursor(prepared = True)
                    query = "INSERT INTO Delivery(start_time, origin, state, sender, receiver) VALUES (%s, %s, 0, %s, %s)"
                    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    values = (timestamp, data["origin"], email, data["email"],)
                    try:
                        cursor.execute(query, values)
                        connection.commit()
                        query = "SELECT id FROM Delivery WHERE start_time = %s AND origin = %s AND state = 0 AND sender = %s AND receiver = %s"
                        values = (timestamp, data["origin"], email, data["email"],)
                        try:
                            cursor.execute(query, values)
                            result = cursor.fetchone()
                            if result:
                                id = result[0]
                                for login in cookies:
                                    data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/request", "message_body": {"id": id, "sender": email, "origin": data["origin"]}}
                                    await open_sockets["users"][login[0]].send(json.dumps(data2))
                                resp["message_body"] = "true"
                                await websocket.send(json.dumps(resp))
                            else:
                                resp["message_body"] = "false"
                                await websocket.send(json.dumps(resp))
                        except mysql.connector.Error as e:
                            print("request.py:48: " + str(e))
                            resp["message_body"] = "false"
                            await websocket.send(json.dumps(resp))
                    except mysql.connector.Error as e:
                        print("request.py:52: " + str(e))
                        connection.rollback()
                        resp["message_body"] = "false"
                        await websocket.send(json.dumps(resp))
            else:
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
