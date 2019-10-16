import json
import connect
import mysql.connector
import os
import hashlib

def main(handler):
    if handler.command == "POST":
        if "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "email" in data and "first_name" in data and "last_name" in data and "password" in data:
                handler.send_response(200)
                handler.end_headers()
                connection = connect.connect()
                cursor = connection.cursor(prepared = True)
                query = "INSERT INTO User(email, verified, first_name, last_name, salt, hash) VALUES (%s, %s, %s, %s, %s, %s)"
                salt = os.urandom(32)
                hash = hashlib.pbkdf2_hmac("sha256", data["password"].encode("utf-8"), salt, 100000)
                values = (data["email"], 0, data["first_name"], data["last_name"], salt.hex(), hash.hex(),)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    handler.wfile.write("true".encode("utf-8"))
                except mysql.connector.Error:
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
