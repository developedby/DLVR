import json
import asyncio

async def main(websocket, path, data = None):
    if not data:
        data = await websocket.recv()
        data = json.loads(data)
    if "id" in data and "signature" in data and "timestamp" in data:
        resp = {
            "status_code": 200,
            "reason_message": "OK"
        }
        connection = connect.connect()
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
                    timestamp = datetime.datetime.fromtimestamp(data["timestamp"])
                    now = datetime.datetime.now()
                    diff = min((now - timestamp).seconds, (timestamp - now).seconds)
                    if diff < 60:
                        resp["message_body"] = "true"
                        await websocket.send(json.dumps(resp))
                    else:
                        resp["message_body"] = "false"
                        await websocket.send(json.dumps(resp))
                else:
                    resp["message_body"] = "false"
                    await websocket.send(json.dumps(resp))
            else:
                resp["message_body"] = "false"
                await websocket.send(json.dumps(resp))
        except mysql.connector.Error as e:
            print("route.py:43: " + str(e))
            resp["message_body"] = "false"
            await websocket.send(json.dumps(resp))
        cursor.close()
        connection.close()
    else:
        await websocket.send("{\"status_code\": 400, \"reason_message\": \"Bad Request\"}")
