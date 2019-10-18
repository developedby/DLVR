import json
import connect
import mysql.connector
import os
import hashlib
import client

def main(handler):
    if handler.command == "POST":
        if "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "email" in data and "code" in data and "password" in data:
                handler.send_response(200)
                handler.end_headers()
                resp = client.sendJSON("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com/code/verify", {"email": data["email"], "number": data["code"]})
                if resp.read().decode("utf-8") == "true":
                    connection = connect.connect()
                    cursor = connection.cursor(prepared = True)
                    query = "UPDATE User SET salt = %s, hash = %s WHERE email = %s"
                    salt = os.urandom(32)
                    hash = hashlib.pbkdf2_hmac("sha256", data["password"].encode("utf-8"), salt, 100000)
                    values = (salt.hex(), hash.hex(), data["email"],)
                    try:
                        cursor.execute(query, values)
                        connection.commit()
                        handler.wfile.write("true".encode("utf-8"))
                    except mysql.connector.Error:
                        connection.rollback()
                        handler.wfile.write("false".encode("utf-8"))
                    cursor.close()
                    connection.close()
                else:
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
