import json
import session

def main(handler):
    if handler.command == "POST":
        if "Content-Type" in handler.headers and handler.headers["Content-Type"] == "application/json":
            data = json.loads(handler.rfile.read(int(handler.headers["Content-Length"])).decode("utf-8"))
            if "email" in data and "password" in data:
                handler.send_response(200)
                cookie = session.signin(data["email"], data["password"])
                if cookie:
                    handler.send_header("Set-Cookie", cookie.output(header = "", sep = ""))
                    handler.end_headers()
                    handler.wfile.write("true".encode("utf-8"))
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
