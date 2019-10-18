import urllib.request
import ssl
import json

def sendJSON(url, data_dict = None, method = "POST", cookie = None):
    headers = {}
    data = None
    if data_dict:
        data = json.dumps(data_dict).encode("utf-8")
        headers["Content-Type"] = "application/json"
    if cookie:
        headers["Cookie"] = cookie
    req = urllib.request.Request(url, data, headers = headers, method = method)
    resp = urllib.request.urlopen(req, context = ssl._create_unverified_context())
    return resp

def main():
    data = {"email": "subject@example.com", "password": "Password123!", "first_name": "Subject", "last_name": "Subject"}
    resp = sendJSON("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com/user/signup", data)
    if resp.read().decode("utf-8") == "true":
        print("Signup efetuado com sucesso")
        code = int(input())
        data = {"email": "subject@example.com", "number": code}
        resp = sendJSON("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com/code/verify", data)
        if resp.read().decode("utf-8") == "true":
            print("Código verificado com sucesso")
            data = {"email": "subject@example.com"}
            sendJSON("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com/code/generate", data)
            code = int(input())
            data = {"email": "subject@example.com", "code": code, "password": "Password456!"}
            resp = sendJSON("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com/user/reset", data)
            if resp.read().decode("utf-8") == "true":
                print("Senha redefinida com sucesso")
                data = {"email": "subject@example.com", "password": "Password456!"}
                resp = sendJSON("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com/user/signin", data)
                if resp.read().decode("utf-8") == "true":
                    print("Signin efetuado com sucesso")
                    cookie = resp.getheader("Set-Cookie")
                    print("Cookie: {}".format(cookie))
                    data = {"password": "Password456!"}
                    resp = sendJSON("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com/user/delete", data, cookie = cookie)
                    if resp.read().decode("utf-8") == "true":
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
