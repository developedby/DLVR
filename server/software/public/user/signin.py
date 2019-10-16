import json
import connect
import mysql.connector
import hashlib
import http.cookies
import os

def main(handler):
    if handler.command == "POST":
        if "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "email" in data and "password" in data:
                handler.send_response(200)
                connection = connect.connect()
                cursor = connection.cursor(prepared = True)
                query = "SELECT salt, hash FROM User WHERE email = %s"
                values = (data["email"],)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        salt = bytes.fromhex(result[0])
                        hash = hashlib.pbkdf2_hmac("sha256", data["password"].encode("utf-8"), salt, 100000)
                        if hash == bytes.fromhex(result[1]):
                            cookie = http.cookies.SimpleCookie()
                            cookie["token"] = os.urandom(32).hex()
                            query = "INSERT INTO Login(cookie, email) VALUES (%s, %s)"
                            values = (cookie["token"].value, data["email"],)
                            try:
                                cursor.execute(query, values)
                                connection.commit()
                                resp = "true"
                                handler.send_header("Set-Cookie", cookie.output(header = "", sep = ""))
                            except mysql.connector.Error:
                                resp = "false"
                        else:
                            resp = "false"
                    else:
                        resp = "false"
                    handler.end_headers()
                    handler.wfile.write(resp.encode("utf-8"))
                except mysql.connector.Error:
                    handler.end_headers()
                    handler.wfile.write("false".encode("utf-8"))
                cursor.close()
                connection.close()
            else:
                handler.send_error(400)
                handler.end_headers()
        else:
            handler.send_error(400)
            handler.end_headers()
    else:
        handler.send_error(501)
        handler.end_headers()
