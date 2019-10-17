import json
import connect
import mysql.connector
import os
import datetime
import random

def main(handler):
    if handler.command == "POST":
        if "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "email" in data and "number" in data:
                handler.send_response(200)
                handler.end_headers()
                connection = connect.connect()
                cursor = connection.cursor(prepared = True)
                query = "DELETE FROM Code WHERE expiration < %s"
                timestamp = datetime.datetime.fromtimestamp(datetime.datetime.now().timestamp()).strftime("%Y-%m-%d %H:%M:%S")
                values = (timestamp,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    query = "SELECT user FROM Code WHERE number = %s"
                    values = (data["number"],)
                    try:
                        cursor.execute(query, values)
                        result = cursor.fetchone()
                        if result and result[0] == data["email"]:
                            query = "DELETE FROM Code WHERE number = %s"
                            values = (data["number"],)
                            try:
                                cursor.execute(query, values)
                                query = "UPDATE User SET verified = 1 WHERE email = %s"
                                values = (data["email"],)
                                try:
                                    cursor.execute(query, values)
                                    connection.commit()
                                    handler.wfile.write("true".encode("utf-8"))
                                except mysql.connector.Error:
                                    connection.rollback()
                                    handler.wfile.write("false".encode("utf-8"))
                            except mysql.connector.Error:
                                connection.rollback()
                                handler.wfile.write("false".encode("utf-8"))
                        else:
                            handler.wfile.write("false".encode("utf-8"))
                    except mysql.connector.Error:
                        connection.rollback()
                        handler.wfile.write("false".encode("utf-8"))
                except mysql.connector.Error:
                    connection.rollback()
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
