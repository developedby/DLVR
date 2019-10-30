import asyncio
import websockets
import ssl
import sys
import os
import mimetypes
import importlib
import datetime

script_cache = {}

def main():
    try:
        PORT = 443
        ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ssl_context.load_cert_chain("cert.pem", "key.pem")
        wssd = websockets.serve(handler, "", PORT, ssl = ssl_context)
        print("Serving WSS on 0.0.0.0 port {0} (wss://0.0.0.0:{0}/) ...".format(PORT))
        sys.path.append(os.getcwd())
        os.chdir("./public")
        asyncio.get_event_loop().run_until_complete(wssd)
        asyncio.get_event_loop().run_forever()
    except KeyboardInterrupt as e:
        print("websocketserver.py:23: " + str(e))

async def handler(websocket, path):
    print("{0} - - [{1}] \"{2}\"".format(websocket.remote_address[0], datetime.datetime.now().strftime("%d/%b/%Y %H:%M:%S"), path))

    local_path = "./" + path.strip("/")

    if not os.path.exists(local_path):
        local_path += ".py"
    elif not os.path.isfile(local_path):
        local_path += "index.py"

    mimetype = mimetypes.guess_type(local_path)

    if os.path.exists(local_path) and mimetype[0] == "text/x-python":
        module_name = "public." + local_path.replace(".py", "").replace("/", ".").strip(".")
        if module_name in script_cache:
            if script_cache[module_name]["lastmodified"] < os.path.getmtime(local_path):
                script_cache[module_name]["module"] = importlib.reload(script_cache[module_name]["module"])
                script_cache[module_name]["lastmodified"] = os.path.getmtime(local_path)

            script = script_cache[module_name]["module"]
        else:
            script = importlib.import_module(module_name)
            script_cache[module_name] = {
                "module": script,
                "lastmodified": os.path.getmtime(local_path)
            }

        try:
            await script.main(websocket, path)
        except Exception as e:
            print("websocketserver.py:53(" + script.__name__ + "): " + str(e))
            await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
    else:
        await websocket.send("{\"status_code\": 404, \"reason_message\": \"Not Found\"}")

if __name__ == "__main__":
    main()
