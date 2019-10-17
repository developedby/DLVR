import session
import http.cookies

def main(handler):
    if handler.command == "POST":
        if "Cookie" in handler.headers:
            handler.send_response(200)
            handler.end_headers()
            cookie = http.cookies.SimpleCookie()
            cookie.load(handler.headers["Cookie"])
            email = session.user_email(cookie["token"].value)
            if email:
                session.signout(cookie["token"].value)
                handler.wfile.write("true".encode("utf-8"))
            else:
                handler.wfile.write("false".encode("utf-8"))
        else:
            handler.send_error(400)
            handler.end_headers()
    else:
        handler.send_error(501)
        handler.end_headers()
