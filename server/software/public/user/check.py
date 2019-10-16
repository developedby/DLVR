import json
import connect
import mysql.connector

def main(handler):
    if handler.command == "POST":
        if "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "email" in data:
                handler.send_response(200)
                handler.end_headers()
                connection = connect.connect()
                cursor = connection.cursor(prepared = True)
                query = "SELECT email FROM User WHERE email = %s"
                values = (data["email"],)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    resp = "true" if result else "false"
                    handler.wfile.write(resp.encode("utf-8"))
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
