import json
import session
import connect
import mysql.connector
import hashlib
import http.cookies
import os

def main(handler):
    if handler.command == "POST":
        if "Cookie" in handler.headers and "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "password" in data and ("email" in data or "first_name" in data or "last_name" in data or "new_password" in data):
                handler.send_response(200)
                cookie = http.cookies.SimpleCookie()
                cookie.load(handler.headers["Cookie"])
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
                                        handler.send_header("Set-Cookie", cookie.output(header = "", sep = ""))
                                        email = data["email"]
                                    except mysql.connector.Error:
                                        errors += 1
                                if "first_name" in data:
                                    query = "UPDATE User SET first_name = %s WHERE email = %s"
                                    values = (data["first_name"], email,)
                                    try:
                                        cursor.execute(query, values)
                                    except mysql.connector.Error:
                                        errors += 1
                                if "last_name" in data:
                                    query = "UPDATE User SET last_name = %s WHERE email = %s"
                                    values = (data["last_name"], email,)
                                    try:
                                        cursor.execute(query, values)
                                    except mysql.connector.Error:
                                        errors += 1
                                if "new_password" in data:
                                    query = "UPDATE User SET salt = %s, hash = %s WHERE email = %s"
                                    salt = os.urandom(32)
                                    hash = hashlib.pbkdf2_hmac("sha256", data["new_password"].encode("utf-8"), salt, 100000)
                                    values = (salt.hex(), hash.hex(), email,)
                                    try:
                                        cursor.execute(query, values)
                                    except mysql.connector.Error:
                                        errors += 1
                                if errors == 0:
                                    connection.commit()
                                    handler.end_headers()
                                    handler.wfile.write("true".encode("utf-8"))
                                else:
                                    connection.rollback()
                                    handler.end_headers()
                                    handler.wfile.write("false".encode("utf-8"))
                            else:
                                handler.end_headers()
                                handler.wfile.write("false".encode("utf-8"))
                        else:
                            handler.end_headers()
                            handler.wfile.write("false".encode("utf-8"))
                    except mysql.connector.Error:
                        handler.end_headers()
                        handler.wfile.write("false".encode("utf-8"))
                    cursor.close()
                    connection.close()
                else:
                    handler.end_headers()
                    handler.wfile.write("false".encode("utf-8"))
            else:
                handler.send_error(400)
                handler.end_headers()
        else:
            handler.send_error(400)
            handler.end_headers()
    else:
        handler.send_error(501)
        handler.end_headers()
