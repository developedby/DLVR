import json
import session
import connect
import mysql.connector
import hashlib
import http.cookies

def main(handler):
    if handler.command == "POST":
        if "Cookie" in handler.headers and "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "password" in data:
                handler.send_response(200)
                handler.end_headers()
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
                                query = "DELETE FROM User WHERE email = %s"
                                values = (email,)
                                try:
                                    session.signout(cookie["token"].value)
                                    cursor.execute(query, values)
                                    connection.commit()
                                    handler.wfile.write("true".encode("utf-8"))
                                except mysql.connector.Error:
                                    connection.rollback()
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
