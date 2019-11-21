import random
import datetime
import Crypto.PublicKey.RSA
import os
import hashlib
import json
import math
import logging
import email.mime.multipart
import email.mime.text
import smtplib
import importlib
import enum

def full_class_name(o):
    module = o.__class__.__module__
    if module == None or module == str.__class__.__module__:
        return o.__class__.__name__
    else:
        return module + "." + o.__class__.__name__

def init_logging(filename):
    logging.basicConfig(filename = filename, format = "%(message)s", level = logging.INFO)

def send_email(email_address, first_name, last_name, number):
    try:
        msg = email.mime.multipart.MIMEMultipart("alternative")
        msg["From"] = "projectdlvr@gmail.com"
        msg["To"] = email_address
        msg["Subject"] = "DLVR Verification Code"
        with open("/home/ec2-user/software/email.txt", "r") as text_file:
            text = text_file.read()
        with open("/home/ec2-user/software/email.html", "r") as html_file:
            html = html_file.read()
        text = text.replace("{{first_name}}", first_name)
        text = text.replace("{{last_name}}", last_name)
        text = text.replace("{{number}}", str(number))
        html = html.replace("{{first_name}}", first_name)
        html = html.replace("{{last_name}}", last_name)
        html = html.replace("{{number}}", str(number))
        part1 = email.mime.text.MIMEText(text, "plain")
        part2 = email.mime.text.MIMEText(html, "html")
        msg.attach(part1)
        msg.attach(part2)
        with open("/home/ec2-user/software/password", "r") as pwd_file:
            password = pwd_file.read()
        server = smtplib.SMTP("smtp.gmail.com:587")
        server.starttls()
        server.login(msg["From"], password)
        server.sendmail(msg["From"], msg["To"], msg.as_string())
        server.quit()
        return True
    except Exception as e:
        module.error(e)
    return False

class Direction(enum.Enum):
    FRONT = 0
    LEFT = 1
    BACK = 2
    RIGHT = 3

    @classmethod
    def fromstr(cls, str):
        if str == 'front':
            return cls(0)
        elif str == 'left':
            return cls(1)
        elif str == 'back':
            return cls(2)
        elif str == 'right':
            return cls(3)

    def __str__(self):
        if int(self) == 0:
            return 'front'
        elif int(self) == 1:
            return 'left'
        elif int(self) == 2:
            return 'back'
        elif int(self) == 3:
            return 'right'

    def __int__(self):
        return self.value

    def __eq__(self, other):
        return int(self) == int(other) % 4

    def __neg__(self):
        return Direction(-int(self) % 4)

    def __add__(self, other):
        return Direction((int(self) + int(other)) % 4)

    def __sub__(self, other):
        return self + (-other)

    def __radd__(self, other):
        return self + other

    def __rsub__(self, other):
        return (-self) + other

city = {
    1: {2: (Direction.RIGHT, 0.72)},
    2: {1: (Direction.LEFT, 0.72), 3: (Direction.RIGHT, 0.26), 10: (Direction.BACK, 0.77)},
    3: {2: (Direction.LEFT, 0.26), 4: (Direction.RIGHT, 0.71)},
    4: {3: (Direction.LEFT, 0.71)},
    5: {3: (Direction.FRONT, 0.25), 6: (Direction.RIGHT, 0.46)},
    6: {7: (Direction.RIGHT, 0.25), 8: (Direction.BACK, 0.26)},
    7: {6: (Direction.LEFT, 0.25)},
    8: {9: (Direction.RIGHT, 0.25), 12: (Direction.BACK, 0.26)},
    9: {8: (Direction.LEFT, 0.25)},
    10: {24: (Direction.LEFT, 0.31), 16: (Direction.BACK, 0.52)},
    11: {10: (Direction.LEFT, 0.26), 5: (Direction.FRONT, 0.52)},
    12: {11: (Direction.LEFT, 0.46), 13: (Direction.RIGHT, 0.25)},
    13: {12: (Direction.LEFT, 0.25)},
    14: {12: (Direction.FRONT, 0.26), 15: (Direction.RIGHT, 0.25)},
    15: {14: (Direction.LEFT, 0.25)},
    16: {21: (Direction.BACK, 0.39), 17: (Direction.RIGHT, 0.26)},
    17: {11: (Direction.FRONT, 0.52), 18: (Direction.RIGHT, 0.46)},
    18: {14: (Direction.FRONT, 0.26), 19: (Direction.RIGHT, 0.25)},
    19: {18: (Direction.LEFT, 0.25)},
    20: {21: (Direction.RIGHT, 0.72)},
    21: {20: (Direction.LEFT, 0.72), 22: (Direction.RIGHT, 0.26)},
    22: {17: (Direction.FRONT, 0.39), 21: (Direction.LEFT, 0.26), 23: (Direction.RIGHT, 0.72)},
    23: {22: (Direction.LEFT, 0.72)},
    24: {25: (Direction.LEFT, 0.34)},
    25: {26: (Direction.LEFT, 0.21)},
    26: {30: (Direction.BACK, 0.54)},
    27: {28: (Direction.RIGHT, 0.20)},
    28: {29: (Direction.RIGHT, 0.25)},
    29: {16: (Direction.RIGHT, 0.31)},
    30: {27: (Direction.BACK, 0.36), 31: (Direction.RIGHT, 0.24)},
    31: {30: (Direction.LEFT, 0.24)},
}

garages = [1, 4, 20, 23]

def shortest_path(graph, start, goal):
    queue = [(start, [start])]
    while queue:
        (vertex, path) = queue.pop(0)
        for next in set(graph[vertex]) - set(path):
            if next == goal:
                return path + [next]
            else:
                queue.append((next, path + [next]))

def path_to_directions(graph, path, orientation):
    ret = []
    for origin, destination in zip(path[:-1], path[1:]):
        direction = graph[origin][destination][0] - orientation
        orientation = graph[origin][destination][0]
        ret.append((str(direction), graph[origin][destination][1], destination))
    return ret

class ScriptCache:
    def __init__(self):
        self.script_cache = {}

    def import_(self, local_path):
        local_path = "./" + local_path.strip("/")
        if not os.path.exists(local_path):
            local_path += ".py"
        elif not os.path.isfile(local_path):
            local_path += "index.py"
        module_name = "public." + local_path.replace(".py", "").replace("/", ".").strip(".")
        if module_name in self.script_cache:
            if self.script_cache[module_name]["lastmodified"] < os.path.getmtime(local_path):
                self.script_cache[module_name]["module"] = importlib.reload(self.script_cache[module_name]["module"])
                self.script_cache[module_name]["lastmodified"] = os.path.getmtime(local_path)

            script = self.script_cache[module_name]["module"]
        else:
            script = importlib.import_module(module_name)
            self.script_cache[module_name] = {
                "module": script,
                "lastmodified": os.path.getmtime(local_path)
            }
        return script

class Request:
    def __init__(self, websocket):
        self.websocket = websocket

    def log(self, path, message = None):
        if message != None:
            log_message = "[{}] - - {}:{} \"{}\": {}".format(datetime.datetime.now().strftime("%d/%b/%Y %H:%M:%S"), self.websocket.remote_address[0], self.websocket.remote_address[1], path, message)
        else:
            log_message = "[{}] - - {}:{} \"{}\"".format(datetime.datetime.now().strftime("%d/%b/%Y %H:%M:%S"), self.websocket.remote_address[0], self.websocket.remote_address[1], path)
        print(log_message)
        logging.info(log_message)

    def error(self, path, e):
        self.log(path, "{}: {}".format(full_class_name(e), e))

class Module:
    def __init__(self, module):
        self.module = module

    def log(self, message, submodule = None):
        if submodule != None:
            log_message = "[{}] {}({}): {}".format(datetime.datetime.now().strftime("%d/%b/%Y %H:%M:%S"), self.module, submodule, message)
        else:
            log_message = "[{}] {}: {}".format(datetime.datetime.now().strftime("%d/%b/%Y %H:%M:%S"), self.module, message)
        print(log_message)
        logging.info(log_message)

    def error(self, e, submodule = None):
        self.log("{}: {}".format(full_class_name(e), e), submodule)

module = Module(__name__)

import connect

class Password:
    @classmethod
    def create(cls, password):
        salt = os.urandom(32)
        hash = hashlib.pbkdf2_hmac("sha256", password.encode("utf-8"), salt, 100000)
        return cls(salt, hash)

    def __init__(self, salt, hash):
        self.salt = salt
        self.hash = hash

    def __eq__(self, other):
        hash = hashlib.pbkdf2_hmac("sha256", other.encode("utf-8"), self.salt, 100000)
        if hash == self.hash:
            return True
        else:
            return False

    def __ne__(self, other):
        return not (self == other)

class User:
    @classmethod
    def create(cls, email, first_name, last_name, password):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO User(email, verified, first_name, last_name, salt, hash) VALUES (%s, false, %s, %s, %s, %s)"
            password = Password.create(password)
            values = (email, first_name, last_name, password.salt.hex(), password.hash.hex())
            try:
                cursor.execute(query, values)
                connection.commit()
                return cls(email)
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    def __init__(self, email):
        self._email = email

    @property
    def email(self):
        return self._email

    @email.setter
    def email(self, value):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE User SET email = %s WHERE email = %s"
                values = (value, self._email)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    self._email = value
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()

    @property
    def verified(self):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT verified FROM User WHERE email = %s"
                values = (self._email,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return bool(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @verified.setter
    def verified(self, value):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE User SET verified = %s WHERE email = %s"
                values = (value, self._email)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()

    @property
    def first_name(self):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT first_name FROM User WHERE email = %s"
                values = (self._email,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return result[0]
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @first_name.setter
    def first_name(self, value):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE User SET first_name = %s WHERE email = %s"
                values = (value, self._email)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()

    @property
    def last_name(self):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT last_name FROM User WHERE email = %s"
                values = (self._email,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return result[0]
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @last_name.setter
    def last_name(self, value):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE User SET last_name = %s WHERE email = %s"
                values = (value, self._email)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()

    @property
    def password(self):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT salt, hash FROM User WHERE email = %s"
                values = (self._email,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return Password(bytes.fromhex(result[0]), bytes.fromhex(result[1]))
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @password.setter
    def password(self, value):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE User SET salt = %s, hash = %s WHERE email = %s"
                value = Password.create(value)
                values = (value.salt.hex(), value.hash.hex(), self._email)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()

    @property
    def code(self):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT number FROM Code WHERE user = %s"
                values = (self._email,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return Code(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def logins(self):
        if self._email != None:
            ret = []
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT cookie FROM Login WHERE user = %s"
                values = (self._email,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchall()
                    if result:
                        for login in result:
                            ret.append(Login(login[0]))
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()
            return ret

    @property
    def deliveries(self):
        if self._email != None:
            ret = []
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT DISTINCT id FROM Delivery WHERE sender = %s OR receiver = %s"
                values = (self._email, self._email)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchall()
                    if result:
                        for delivery in result:
                            ret.append(Delivery(delivery[0]))
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()
            return ret

    @property
    def delivery(self):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT DISTINCT id FROM Delivery WHERE (sender = %s OR receiver = %s) AND state < 5"
                values = (self._email, self._email)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return Delivery(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    def delete(self):
        if self._email != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "DELETE FROM User WHERE email = %s"
                values = (self._email,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    self._email = None
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

class Robot:
    @classmethod
    def verify(cls, public_key, data):
        data_no_sign = data.copy()
        data_no_sign.pop("signature")
        data_no_sign = json.dumps(data_no_sign, sort_keys = True).encode("utf-8")
        hash = hashlib.sha256(data_no_sign).hexdigest().encode("utf-8")
        if public_key.verify(hash, (data["signature"],)):
            timestamp = datetime.datetime.fromtimestamp(data["timestamp"])
            now = datetime.datetime.now()
            diff = min((now - timestamp).seconds, (timestamp - now).seconds)
            if diff < 60:
                return True
        return False

    @classmethod
    def choose(cls, position):
        available = cls.available_robots()
        nearest = None
        if available and len(available) > 0:
            for robot in available:
                route = shortest_path(city, robot.position, position)
                if nearest == None or len(route) < len(nearest[1]):
                    nearest = (robot, route)
        return nearest

    @classmethod
    def available_robots(cls):
        ret = []
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT id FROM Robot WHERE alive = true AND state = 6"
            try:
                cursor.execute(query)
                result = cursor.fetchall()
                if result:
                    for robot in result:
                        ret.append(cls(robot[0]))
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()
        return ret

    def __init__(self, id):
        self._id = id

    @property
    def id(self):
        return self._id

    @property
    def public_key(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT public_key FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return Crypto.PublicKey.RSA.importKey(bytes.fromhex(result[0]))
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @property
    def alive(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT alive FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return bool(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @alive.setter
    def alive(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET alive = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def position(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT position FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @position.setter
    def position(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET position = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def orientation(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT orientation FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return Direction(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @orientation.setter
    def orientation(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET orientation = %s WHERE id = %s"
            values = (int(value), self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def route(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT route FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return json.loads(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @route.setter
    def route(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET route = %s WHERE id = %s"
            values = (json.dumps(value), self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def item(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT item FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return bool(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @item.setter
    def item(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET item = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def speed(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT speed FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @speed.setter
    def speed(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET speed = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def curve_radius(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT curve_radius FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @curve_radius.setter
    def curve_radius(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET curve_radius = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def left_encoder(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT left_encoder FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @left_encoder.setter
    def left_encoder(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET left_encoder = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def right_encoder(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT right_encoder FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @right_encoder.setter
    def right_encoder(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET right_encoder = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def ultrasound(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT ultrasound FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @ultrasound.setter
    def ultrasound(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET ultrasound = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def state(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT state FROM Robot WHERE id = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    @state.setter
    def state(self, value):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET state = %s WHERE id = %s"
            values = (value, self._id)
            try:
                cursor.execute(query, values)
                connection.commit()
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    @property
    def deliveries(self):
        ret = []
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT id FROM Delivery WHERE robot = %s"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchall()
                if result:
                    for delivery in result:
                        ret.append(Delivery(delivery[0]))
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()
        return ret

    @property
    def delivery(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT id FROM Delivery WHERE robot = %s AND state < 5"
            values = (self._id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return Delivery(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

class Code:
    @classmethod
    def delete_expired(cls, user):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM Code WHERE expiration < %s"
            expiration = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (expiration,)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "DELETE FROM Code WHERE user = %s"
                values = (user,)
                cursor.execute(query, values)
                connection.commit()
                return True
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()
        return False

    @classmethod
    def create(cls, user):
        cls.delete_expired(user)
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO Code(number, expiration, user) VALUES (%s, %s, %s)"
            number = random.randint(0, 65535)
            expiration = datetime.datetime.now() + datetime.timedelta(minutes = 5)
            expiration = expiration.strftime("%Y-%m-%d %H:%M:%S")
            values = (number, expiration, user)
            try:
                cursor.execute(query, values)
                connection.commit()
                return cls(number)
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    def __init__(self, number):
        self._number = number

    @property
    def number(self):
        return self._number

    @property
    def expiration(self):
        if self._number != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT expiration FROM Code WHERE number = %s"
                values = (self._number,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return datetime.datetime.fromtimestamp(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def user(self):
        if self._number != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT user FROM Code WHERE number = %s"
                values = (self._number,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return User(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    def delete(self):
        if self._number != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "DELETE FROM Code WHERE number = %s"
                values = (self._number,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    self._number = None
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

class Login:
    @classmethod
    def create(cls, user):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO Login(cookie, user) VALUES (%s, %s)"
            cookie = os.urandom(32).hex()
            values = (cookie, user)
            try:
                cursor.execute(query, values)
                connection.commit()
                return cls(cookie)
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    def __init__(self, cookie):
        self._cookie = cookie

    @property
    def cookie(self):
        return self._cookie

    @property
    def user(self):
        if self._cookie != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT user FROM Login WHERE cookie = %s"
                values = (self._cookie,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return User(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    def delete(self):
        if self._cookie != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "DELETE FROM Login WHERE cookie = %s"
                values = (self._cookie,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    self._cookie = None
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

class Delivery:
    @classmethod
    def next_id(cls):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT MAX(id) FROM Delivery"
            try:
                cursor.execute(query)
                result = cursor.fetchone()
                if result and result[0] != None:
                    return result[0] + 1
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()
        return 0

    @classmethod
    def create(cls, origin, sender, receiver):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO Delivery(id, start_time, origin, state, sender, receiver, path) VALUES (%s, %s, %s, 0, %s, %s, '[]')"
            id = cls.next_id()
            start_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (id, start_time, origin, sender, receiver)
            try:
                cursor.execute(query, values)
                connection.commit()
                return cls(id)
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    def __init__(self, id):
        self._id = id

    @property
    def id(self):
        return self._id

    @property
    def start_time(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT start_time FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return datetime.datetime.fromtimestamp(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def finish_time(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT finish_time FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return datetime.datetime.fromtimestamp(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def origin(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT origin FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return result[0]
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def destination(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT destination FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return result[0]
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def path(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT path FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return json.loads(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @path.setter
    def path(self, value):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE Delivery SET path = %s WHERE id = %s"
                values = (json.dumps(value), self._id)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()

    @property
    def state(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT state FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return result[0]
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def sender(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT sender FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return User(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def receiver(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT receiver FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return User(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def robot(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT robot FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return Robot(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def qr_code(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT number FROM QRCode WHERE delivery = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return QRCode(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    def response(self, position, destination, robot):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE Delivery SET state = 1, path = %s, destination = %s, robot = %s WHERE id = %s"
                values = (json.dumps([position]), destination, robot, self._id)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

    def origin_arrival(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE Delivery SET state = 2 WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

    def send(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE Delivery SET state = 3 WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

    def qr(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE Delivery SET state = 4 WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

    def finish(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "UPDATE Delivery SET state = 5, finish_time = %s WHERE id = %s"
                finish_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                values = (finish_time, self._id)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

    def delete(self):
        if self._id != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "DELETE FROM Delivery WHERE id = %s"
                values = (self._id,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    self._id = None
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False

class QRCode:
    @classmethod
    def delete_expired(cls, delivery):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM QRCode WHERE expiration < %s"
            expiration = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (expiration,)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "DELETE FROM QRCode WHERE delivery = %s"
                values = (delivery,)
                cursor.execute(query, values)
                connection.commit()
                return True
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()
        return False

    @classmethod
    def create(cls, delivery):
        cls.delete_expired(delivery)
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO QRCode(number, expiration, delivery) VALUES (%s, %s, %s)"
            number = random.randint(32, 65535)
            expiration = datetime.datetime.now() + datetime.timedelta(minutes = 5)
            expiration = expiration.strftime("%Y-%m-%d %H:%M:%S")
            values = (number, expiration, delivery)
            try:
                cursor.execute(query, values)
                connection.commit()
                return cls(number)
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    def __init__(self, number):
        self._number = number

    @property
    def number(self):
        return self._number

    @property
    def expiration(self):
        if self._number != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT expiration FROM QRCode WHERE number = %s"
                values = (self._number,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return datetime.datetime.fromtimestamp(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    @property
    def delivery(self):
        if self._number != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "SELECT delivery FROM QRCode WHERE number = %s"
                values = (self._number,)
                try:
                    cursor.execute(query, values)
                    result = cursor.fetchone()
                    if result:
                        return Delivery(result[0])
                except Exception as e:
                    module.error(e)
                finally:
                    cursor.close()

    def delete(self):
        if self._number != None:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "DELETE FROM QRCode WHERE number = %s"
                values = (self._number,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    self._number = None
                    return True
                except Exception as e:
                    module.error(e)
                    connection.rollback()
                finally:
                    cursor.close()
        return False
