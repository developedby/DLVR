import json
import connect
import mysql.connector

def main(handler):
    if handler.command == "POST":
        if "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "id" in data and "signature" in data and "timestamp" in data:
                handler.send_response(200)
                handler.end_headers()
                connection = connect.connect()
                cursor = connection.cursor(prepared = True)
                query = "SELECT public_key FROM Robot WHERE id = %s"
                values = (data["id"],)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        public_key = Crypto.PublicKey.importKey(bytes.fromhex(result[0]))
                        data_no_sign = data.copy()
                        data_no_sign.pop("signature")
                        data_no_sign = str(data_no_sign).encode("utf-8")
                        hash = hashlib.sha256(data_no_sign).hexdigest()
                        if public_key.verify(data_no_sign, (int(bytes.fromhex(data["signature"])),)):
                            timestamp = datetime.datetime.fromtimestamp(data["timestamp"])
                            now = datetime.datetime.now()
                            diff = min((now - timestamp).seconds, (timestamp - now).seconds)
                            if diff < 60:
                                query = "UPDATE Robot SET alive=true WHERE id = %s"
                                values = (data["id"],)
                                try:
                                    cursor.execute(query, values)
                                    connection.commit()
                                    handler.wfile.write("true".encode("utf-8"))
                                except mysql.connector.Error:
                                    handler.wfile.write("false".encode("utf-8"))
                            else:
                                handler.wfile.write("false".encode("utf-8"))
                        else:
                            handler.wfile.write("false".encode("utf-8"))
                    else:
                        handler.wfile.write("false".encode("utf-8"))
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
