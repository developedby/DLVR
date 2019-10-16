import urllib.request
import ssl
import json

def getFromURL(url, data_dict, method = "POST", cookie = None):
    data = json.dumps(data_dict).encode("utf-8")
    headers = {"Content-Type": "application/json"}
    if cookie:
        headers["Cookie"] = cookie
    req = urllib.request.Request(url, data, headers, method = method)
    resp = urllib.request.urlopen(req, context = ssl._create_unverified_context())
    return resp

def main():
    data = {"email": "subject@example.com", "password": "Password123!"}
    resp = getFromURL("https://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com/user/signin", data)
    print(resp.read().decode("utf-8"))
    print(resp.getheader("Set-Cookie"))

if __name__ == "__main__":
    main()
