import json
import session
import connect
import mysql.connector
import hashlib
import http.cookies
import asyncio

async def main(websocket, path, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "password" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        cookie = http.cookies.SimpleCookie()
        cookie.load(data["cookie"])
        email = session.user_email(cookie["token"].value)
        if email:
            connection = connect.connect()
            cursor = connection.cursor(prepared = True)
            query = "SELECT salt, hash FROM User WHERE email = %s"
            values = (email,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    salt = bytes.fromhex(result[0])
                    hash = hashlib.pbkdf2_hmac("sha256", data["password"].encode("utf-8"), salt, 100000)
                    if hash == bytes.fromhex(result[1]):
                        query = "DELETE FROM User WHERE email = %s"
                        values = (email,)
                        try:
                            session.signout(cookie["token"].value)
                            cursor.execute(query, values)
                            connection.commit()
                            resp["message_body"] = "true"
                            await websocket.send(json.dumps(resp))
                        except mysql.connector.Error as e:
                            print("delete.py:41: " + str(e))
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
                print("delete.py:52: " + str(e))
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
            cursor.close()
            connection.close()
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
