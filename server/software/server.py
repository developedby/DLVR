import http.server
import socketserver
import ssl
import os
import mimetypes
import importlib

def main():
    try:
        PORT = 443
        socketserver.ThreadingTCPServer.allow_reuse_address = True
        httpd = socketserver.ThreadingTCPServer(("", PORT), HTTPRequestHandler)
        httpd.socket = ssl.wrap_socket(httpd.socket, keyfile = "./key.pem", certfile = "./cert.pem", server_side = True)
        print("Serving HTTPS on {0} port {1} (https://{0}:{1}/) ...".format(httpd.server_address[0], httpd.server_address[1]))
        os.chdir("./public")
        httpd.serve_forever()
    except KeyboardInterrupt:
        httpd.server_close()

class HTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    script_cache = {}

    def do_GET(self):
        path = "./" + self.path.strip("/")

        if not os.path.exists(path):
            path += ".py"
        elif not os.path.isfile(path):
            path += "index.py"

        mimetype = mimetypes.guess_type(path)

        if os.path.exists(path) and mimetype[0] == "text/x-python":
            module_name = "public." + path.replace(".py", "").replace("/", ".").strip(".")
            if module_name in self.script_cache:
                if self.script_cache[module_name]["lastmodified"] < os.path.getmtime(path):
                    self.script_cache[module_name]["module"] = importlib.reload(self.script_cache[module_name]["module"])
                    self.script_cache[module_name]["lastmodified"] = os.path.getmtime(path)

                script = self.script_cache[module_name]["module"]
            else:
                script = importlib.import_module(module_name)
                self.script_cache[module_name] = {
                    "module": script,
                    "lastmodified": os.path.getmtime(path)
                }

            try:
                script.main(self)
            except Exception as e:
                print(e)
                self.send_error(500)
                self.end_headers()
        else:
            path = "./" + self.path.strip("/")
            if os.path.isdir(path) and os.path.isfile(path + "/forbidden"):
                self.send_error(403)
                self.end_headers()
            else:
                super().do_GET()

    def do_POST(self):
        path = "./" + self.path.strip("/")

        if not os.path.exists(path):
            path += ".py"
        elif not os.path.isfile(path):
            path += "index.py"

        mimetype = mimetypes.guess_type(path)

        if os.path.exists(path) and mimetype[0] == "text/x-python":
            module_name = "public." + path.replace(".py", "").replace("/", ".").strip(".")
            if module_name in self.script_cache:
                if self.script_cache[module_name]["lastmodified"] < os.path.getmtime(path):
                    self.script_cache[module_name]["module"] = importlib.reload(self.script_cache[module_name]["module"])
                    self.script_cache[module_name]["lastmodified"] = os.path.getmtime(path)

                script = self.script_cache[module_name]["module"]
            else:
                script = importlib.import_module(module_name)
                self.script_cache[module_name] = {
                    "module": script,
                    "lastmodified": os.path.getmtime(path)
                }

            try:
                script.main(self)
            except Exception as e:
                print(e)
                self.send_error(500)
                self.end_headers()
        else:
            self.send_error(501)
            self.end_headers()

    def do_PUT(self):
        path = "./" + self.path.strip("/")

        if not os.path.exists(path):
            path += ".py"
        elif not os.path.isfile(path):
            path += "index.py"

        mimetype = mimetypes.guess_type(path)

        if os.path.exists(path) and mimetype[0] == "text/x-python":
            module_name = "public." + path.replace(".py", "").replace("/", ".").strip(".")
            if module_name in self.script_cache:
                if self.script_cache[module_name]["lastmodified"] < os.path.getmtime(path):
                    self.script_cache[module_name]["module"] = importlib.reload(self.script_cache[module_name]["module"])
                    self.script_cache[module_name]["lastmodified"] = os.path.getmtime(path)

                script = self.script_cache[module_name]["module"]
            else:
                script = importlib.import_module(module_name)
                self.script_cache[module_name] = {
                    "module": script,
                    "lastmodified": os.path.getmtime(path)
                }

            try:
                script.main(self)
            except Exception as e:
                print(e)
                self.send_error(500)
                self.end_headers()
        else:
            self.send_error(501)
            self.end_headers()

    def do_DELETE(self):
        path = "./" + self.path.strip("/")

        if not os.path.exists(path):
            path += ".py"
        elif not os.path.isfile(path):
            path += "index.py"

        mimetype = mimetypes.guess_type(path)

        if os.path.exists(path) and mimetype[0] == "text/x-python":
            module_name = "public." + path.replace(".py", "").replace("/", ".").strip(".")
            if module_name in self.script_cache:
                if self.script_cache[module_name]["lastmodified"] < os.path.getmtime(path):
                    self.script_cache[module_name]["module"] = importlib.reload(self.script_cache[module_name]["module"])
                    self.script_cache[module_name]["lastmodified"] = os.path.getmtime(path)

                script = self.script_cache[module_name]["module"]
            else:
                script = importlib.import_module(module_name)
                self.script_cache[module_name] = {
                    "module": script,
                    "lastmodified": os.path.getmtime(path)
                }

            try:
                script.main(self)
            except Exception as e:
                print(e)
                self.send_error(500)
                self.end_headers()
        else:
            self.send_error(501)
            self.end_headers()

if __name__ == "__main__":
    main()
