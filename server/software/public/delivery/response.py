import json
import session
import connect
import mysql.connector
import http.cookies
import asyncio

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "id" in data and "accept" in data and (not data["accept"] or "destination" in data):
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        cookie = http.cookies.SimpleCookie()
        cookie.load(data["cookie"])
        email = session.user_email(cookie["token"].value)
        if email:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                if data["accept"]:
                    query = "UPDATE Delivery SET state = 1, destination = %s WHERE id = %s AND receiver = %s"
                    values = (data["destination"], data["id"], email,)
                    try:
                        cursor.execute(query, values)
                        connection.commit()
                        query = "SELECT sender FROM Delivery WHERE id = %s"
                        values = (data["id"],)
                        try:
                            cursor.execute(query, values)
                            result = cursor.fetchone()
                            if result:
                                sender = result[0]
                                cookies = session.user_cookies(sender)
                                if len(cookies) > 0:
                                    for login in cookies:
                                        data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"id": data["id"], "accept": True, "destination": data["destination"]}}
                                        await open_sockets["users"][login[0]].send(json.dumps(data2))
                                    resp["message_body"] = "true"
                                    await websocket.send(json.dumps(resp))
                                else:
                                    resp["message_body"] = "false"
                                    await websocket.send(json.dumps(resp))
                            else:
                                resp["message_body"] = "false"
                                await websocket.send(json.dumps(resp))
                        except mysql.connector.Error as e:
                            print("response.py:50: " + str(e))
                            resp["message_body"] = "false"
                            await websocket.send(json.dumps(resp))
                    except mysql.connector.Error as e:
                        print("response.py:54: " + str(e))
                        connection.rollback()
                        resp["message_body"] = "false"
                        await websocket.send(json.dumps(resp))
                else:
                    query = "SELECT sender FROM Delivery WHERE id = %s AND receiver = %s"
                    values = (data["id"], email,)
                    try:
                        cursor.execute(query, values)
                        result = cursor.fetchone()
                        if result:
                            sender = result[0]
                            cookies = session.user_cookies(sender)
                            if len(cookies) > 0:
                                for login in cookies:
                                    data2 = {"status_code": 200, "reason_message": "OK", "path": "/delivery/response", "message_body": {"id": data["id"], "accept": False}}
                                    await open_sockets["users"][login[0]].send(json.dumps(data2))
                                query = "DELETE FROM Delivery WHERE id = %s"
                                values = (data["id"],)
                                try:
                                    cursor.execute(query, values)
                                    connection.commit()
                                    resp["message_body"] = "true"
                                    await websocket.send(json.dumps(resp))
                                except mysql.connector.Error as e:
                                    print("response.py:79: " + str(e))
                                    connection.rollback()
                                    resp["message_body"] = "false"
                                    await websocket.send(json.dumps(resp))
                            else:
                                resp["message_body"] = "false"
                                await websocket.send(json.dumps(resp))
                        else:
                            resp["message_body"] = "false"
                            await websocket.send(json.dumps(resp))
                    except mysql.connector.Error as e:
                        print("response.py:90: " + str(e))
                        resp["message_body"] = "false"
                        await websocket.send(json.dumps(resp))
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
