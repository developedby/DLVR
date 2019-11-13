#!/usr/bin/env python3

import asyncio
import websockets
import ssl
import sys
import os
import mimetypes
import importlib
import datetime
import objects
import public.debug.open
import public.debug.close
import cleanup

script_cache = {}
open_sockets = {"users": {}, "robots": {}}
module = objects.Module("websocketserver")
loop = asyncio.get_event_loop()

async def main():
    HOST = "ec2-18-229-140-84.sa-east-1.compute.amazonaws.com"
    PORT = 443
    ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ssl_context.load_cert_chain("cert.pem", "key.pem")
    wssd = websockets.serve(handler, HOST, PORT, ssl = ssl_context)
    os.system("clear")
    objects.init_logging("websocketserver_{}.log".format(datetime.datetime.now().strftime("%Y_%b_%d")))
    module.log("Serving WSS on {0} port {1} (wss://{0}:{1}/) ...".format(HOST, PORT))
    sys.path.append(os.getcwd())
    os.chdir("./public")
    await wssd
    print("\nEnter a command(help to list all commands):")
    while True:
        print(">>> ", end = "")
        command = await loop.run_in_executor(None, input)
        if command == "help":
            print("help: lists all commands")
            print("clear: clears the screen")
            print("exit: closes the server")
            print("--debug commands--")
            print("open: opens the specified robot")
            print("close: closes the specified robot")
            print("cleanup: clears the database")
        elif command == "clear":
            os.system("clear")
        elif command == "exit":
            module.log("Server closed\n")
            return
        elif command == "open":
            print("Robot id: ", end = "")
            id = int(await loop.run_in_executor(None, input))
            await public.debug.open.main(None, "", open_sockets, {}, id)
            module.log("Robot {} opened".format(id))
        elif command == "close":
            print("Robot id: ", end = "")
            id = int(await loop.run_in_executor(None, input))
            await public.debug.close.main(None, "", open_sockets, {}, id)
            module.log("Robot {} closed".format(id))
        elif command == "cleanup":
            await cleanup.main()
        else:
            print("Invalid command(enter help to list all commands)")


async def handler(websocket, path):
    objects.Request(websocket).log(path)

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
            await script.main(websocket, path, open_sockets)
        except Exception as e:
            module.error(e, script.__name__)
            await websocket.send("{\"status_code\": 500, \"reason_message\": \"Internal Server Error\"}")
    else:
        await websocket.send("{\"status_code\": 404, \"reason_message\": \"Not Found\"}")

if __name__ == "__main__":
    loop.run_until_complete(main())
