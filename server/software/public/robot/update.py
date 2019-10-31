import json
import hashlib
import Crypto.PublicKey.RSA
import connect
import mysql.connector
import asyncio

async def main(websocket, path, open_sockets, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "id" in data and "signature" in data and ("state" in data or "item" in data or "qr" in data or "speed" in data or "curve_radius" in data or "left_encoder" in data or "right_encoder" in data or "ultrasound" in data):
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT public_key FROM Robot WHERE id = %s AND alive = true"
            values = (data["id"],)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    public_key = Crypto.PublicKey.RSA.importKey(bytes.fromhex(result[0]))
                    data_no_sign = data.copy()
                    data_no_sign.pop("signature")
                    data_no_sign = json.dumps(data_no_sign, sort_keys = True).encode("utf-8")
                    hash = hashlib.sha256(data_no_sign).hexdigest().encode("utf-8")
                    if public_key.verify(hash, (data["signature"],)):
                        errors = 0

                        if "state" in data:
                            query = "UPDATE Robot SET state = %s WHERE id = %s"
                            values = (data["state"], data["id"],)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:39: " + str(e))
                                errors += 1

                        if "item" in data:
                            query = "UPDATE Robot SET item = %s WHERE id = %s"
                            values = (data["item"], data["id"],)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:48: " + str(e))
                                errors += 1

                        if "qr" in data:
                            pass

                        if "speed" in data:
                            query = "UPDATE Robot SET speed = %s WHERE id = %s"
                            values = (data["speed"], data["id"],)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:60: " + str(e))
                                errors += 1

                        if "curve_radius" in data:
                            query = "UPDATE Robot SET curve_radius = %s WHERE id = %s"
                            values = (data["curve_radius"], data["id"],)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:69: " + str(e))
                                errors += 1

                        if "left_encoder" in data:
                            query = "UPDATE Robot SET left_encoder = %s WHERE id = %s"
                            values = (data["left_encoder"], data["id"],)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:78: " + str(e))
                                errors += 1

                        if "right_encoder" in data:
                            query = "UPDATE Robot SET right_encoder = %s WHERE id = %s"
                            values = (data["right_encoder"], data["id"],)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:87: " + str(e))
                                errors += 1

                        if "ultrasound" in data:
                            query = "UPDATE Robot SET ultrasound = %s WHERE id = %s"
                            values = (data["ultrasound"], data["id"],)
                            try:
                                cursor.execute(query, values)
                            except mysql.connector.Error as e:
                                print("update.py:96: " + str(e))
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
                print("update.py:114: " + str(e))
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
            cursor.close()
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
