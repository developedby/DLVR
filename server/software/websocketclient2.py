import asyncio
import websockets
import ssl
import json

CERT_PEM_PATH = './cert.pem'
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
ssl_context.load_verify_locations(CERT_PEM_PATH)

def main():
    asyncio.get_event_loop().run_until_complete(handler())

async def handler():
    uri = "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
    data = {"email": "subject2@example.com", "password": "Password456!", "first_name": "Subject2", "last_name": "Subject2"}
    async with websockets.connect(uri + "/user/signup", ssl = ssl_context) as websocket:
        await websocket.send(json.dumps(data))
        resp = await websocket.recv()
        resp = json.loads(resp)
        if resp["status_code"] == 200 and resp["message_body"] == "true":
            print("Signup efetuado com sucesso")
            code = int(input())
            data = {"email": "subject2@example.com", "number": code}
            async with websockets.connect(uri + "/code/verify", ssl = ssl_context) as websocket2:
                await websocket2.send(json.dumps(data))
                resp = await websocket2.recv()
                resp = json.loads(resp)
                if resp["status_code"] == 200 and resp["message_body"] == "true":
                    print("Código verificado com sucesso")
                    data = {"email": "subject2@example.com", "password": "Password456!"}
                    async with websockets.connect(uri + "/user/signin", ssl = ssl_context) as websocket3:
                        await websocket3.send(json.dumps(data))
                        resp = await websocket3.recv()
                        resp = json.loads(resp)
                        if resp["status_code"] == 200 and resp["message_body"] == "true":
                            print("Signin efetuado com sucesso")
                            print("Cookie: {}".format(resp["set_cookie"]))
                            input()
                            data = {"path": "/delivery/request", "cookie": resp["set_cookie"], "origin": 0, "email": "subject@example.com"}
                            await websocket3.send(json.dumps(data))
                            resp = await websocket3.recv()
                            resp = json.loads(resp)
                            if resp["status_code"] == 200 and resp["message_body"] == "true":
                                print("Delivery request efetuado com sucesso")
                            else:
                                print(json.dumps(data))
                                print(resp)
                                print("Delivery request incorreto")
                        else:
                            print(resp)
                            print("Signin incorreto")
                else:
                    print(resp)
                    print("Código incorreto")
        else:
            print(resp)
            print("Signup incorreto")

if __name__ == "__main__":
    main()
