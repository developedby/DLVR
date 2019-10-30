import json
import session
import connect
import mysql.connector
import hashlib
import http.cookies
import os
import asyncio

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "cookie" in data and "password" in data and ("email" in data or "first_name" in data or "last_name" in data or "new_password" in data):
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
                        errors = 0
                        if "email" in data:
                            query = "UPDATE User SET email = %s WHERE email = %s"
                            values = (data["email"], email,)
                            try:
                                session.signout(cookie["token"].value)
                                cursor.execute(query, values)
                                connection.commit()
                                cookie = session.signin(data["email"], data["password"])
                                resp["set_cookie"] = cookie.output(header = "", sep = "")
                                email = data["email"]
                            except mysql.connector.Error as e:
                                print("update.py:45: " + str(e))
                                errors += 1
                        if "first_name" in data:
                            query = "UPDATE User SET first_name = %s WHERE email = %s"
                            values = (data["first_name"], email,)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:53: " + str(e))
                                errors += 1
                        if "last_name" in data:
                            query = "UPDATE User SET last_name = %s WHERE email = %s"
                            values = (data["last_name"], email,)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:61: " + str(e))
                                errors += 1
                        if "new_password" in data:
                            query = "UPDATE User SET salt = %s, hash = %s WHERE email = %s"
                            salt = os.urandom(32)
                            hash = hashlib.pbkdf2_hmac("sha256", data["new_password"].encode("utf-8"), salt, 100000)
                            values = (salt.hex(), hash.hex(), email,)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:71: " + str(e))
                                errors += 1
                        if errors == 0:
                            connection.commit()
                            resp["message_body"] = "true"
                            await websocket.send(json.dumps(resp))
                        else:
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
                print("update.py:88: " + str(e))
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
            cursor.close()
            connection.close()
        else:
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
