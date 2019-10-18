import requests
import ssl
import json

class ServerConnection:
    def __init__(self, base_url):
        self.session = requests.Session()
        self.base_url = base_url

    def send(self, path, data_dict = None, method = "POST"):
        headers = {}
        data = None
        if data_dict:
            data = json.dumps(data_dict).encode("utf-8")
            headers["Content-Type"] = "application/json"
        req = self.session.prepare_request(requests.Request(method, self.base_url + path, data = data, headers = headers))
        resp = self.session.send(req, verify = False)
        return resp

def main():
    server = ServerConnection("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com")
    data = {"email": "subject@example.com", "password": "Password123!", "first_name": "Subject", "last_name": "Subject"}
    resp = server.send("/user/signup", data)
    if resp.content.decode("utf-8") == "true":
        print("Signup efetuado com sucesso")
        code = int(input())
        data = {"email": "subject@example.com", "number": code}
        resp = server.send("/code/verify", data)
        if resp.content.decode("utf-8") == "true":
            print("Código verificado com sucesso")
            data = {"email": "subject@example.com"}
            server.send("/code/generate", data)
            code = int(input())
            data = {"email": "subject@example.com", "code": code, "password": "Password456!"}
            resp = server.send("/user/reset", data)
            if resp.content.decode("utf-8") == "true":
                print("Senha redefinida com sucesso")
                data = {"email": "subject@example.com", "password": "Password456!"}
                resp = server.send("/user/signin", data)
                if resp.content.decode("utf-8") == "true":
                    print("Signin efetuado com sucesso")
                    print("Cookie: {}".format(resp.cookies["token"]))
                    data = {"password": "Password456!"}
                    resp = server.send("/user/delete", data)
                    if resp.content.decode("utf-8") == "true":
                        print("Conta deletada com sucesso")
                    else:
                        print("Erro em deletar a conta")
                else:
                    print("Signin incorreto")
            else:
                print("Erro em redefinir senha")
        else:
            print("Código incorreto")
    else:
        print("Signup incorreto")

if __name__ == "__main__":
    main()
