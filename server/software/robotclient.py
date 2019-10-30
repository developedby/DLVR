import json
import datetime
import hashlib
import Crypto.PublicKey.RSA
import asyncio
import websockets
import ssl

CERT_PEM_PATH = './cert.pem'
ROBOT_DER_PATH = './robot0.der'
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
ssl_context.load_verify_locations(CERT_PEM_PATH)

def main():
    asyncio.get_event_loop().run_until_complete(handler())

async def handler():
    uri = "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
    data = {"id": 0, "timestamp": datetime.datetime.now().timestamp()}
    with open(ROBOT_DER_PATH, "r") as file:
        private_key = Crypto.PublicKey.RSA.importKey(bytes.fromhex(file.read()))
    data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
    async with websockets.connect(uri + "/robot/signin", ssl = ssl_context) as websocket:
        await websocket.send(json.dumps(data))
        resp = await websocket.recv()
        resp = json.loads(resp)
        if resp["status_code"] == 200 and resp["message_body"] == "true":
            print("Signin efetuado com sucesso")
            data = {"id": 0, "timestamp": datetime.datetime.now().timestamp()}
            data["signature"] = private_key.sign(hashlib.sha256(json.dumps(data, sort_keys = True).encode("utf-8")).hexdigest().encode("utf-8"), '')[0]
            async with websockets.connect(uri + "/robot/signout", ssl = ssl_context) as websocket:
                await websocket.send(json.dumps(data))
                resp = await websocket.recv()
                resp = json.loads(resp)
                if resp["status_code"] == 200 and resp["message_body"] == "true":
                    print("Signout efetuado com sucesso")
                else:
                    print(resp)
                    print("Signout incorreto")
        else:
            print(resp)
            print("Signin incorreto")

if __name__ == "__main__":
    main()
