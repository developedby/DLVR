import http.server
import os
import mimetypes
import importlib

def main():
    PORT = 8080
    httpd = http.server.ThreadingHTTPServer(("", PORT), HTTPRequestHandler)
    print("Serving HTTP on {0} port {1} (http://{0}:{1}/) ...".format(httpd.server_address[0], httpd.server_address[1]))
    httpd.serve_forever()

class HTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    script_cache = {}

    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory = "./public", **kwargs)

    def do_GET(self):
        path = "./public" + self.path

        if not os.path.exists(path):
            path += ".py"
        elif not os.path.isfile(path):
            path += "index.py"

        mimetype = mimetypes.guess_type(path)

        if os.path.exists(path) and mimetype[0] == "text/x-python":
            module_name = path.replace(".py", "").replace("/", ".").strip(".")
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

            script.main(self)
        else:
            super().do_GET()

    def do_POST(self):
        path = "./public" + self.path

        if not os.path.exists(path):
            path += ".py"
        elif not os.path.isfile(path):
            path += "/index.py"

        mimetype = mimetypes.guess_type(path)

        if os.path.exists(path) and mimetype[0] == "text/x-python":
            module_name = path.replace(".py", "").replace("/", ".").strip(".")
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

            script.main(self)
        else:
            self.send_response(501)
            self.end_headers()

    def do_PUT(self):
        path = "./public" + self.path

        if not os.path.exists(path):
            path += ".py"
        elif not os.path.isfile(path):
            path += "/index.py"

        mimetype = mimetypes.guess_type(path)

        if os.path.exists(path) and mimetype[0] == "text/x-python":
            module_name = path.replace(".py", "").replace("/", ".").strip(".")
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

            script.main(self)
        else:
            self.send_response(501)
            self.end_headers()

    def do_DELETE(self):
        path = "./public" + self.path

        if not os.path.exists(path):
            path += ".py"
        elif not os.path.isfile(path):
            path += "/index.py"

        mimetype = mimetypes.guess_type(path)

        if os.path.exists(path) and mimetype[0] == "text/x-python":
            module_name = path.replace(".py", "").replace("/", ".").strip(".")
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

            script.main(self)
        else:
            self.send_response(501)
            self.end_headers()

if __name__ == "__main__":
    main()
