import connect
import random
import datetime
import Crypto.PublicKey.RSA
import os
import hashlib
import json
import math

def full_class_name(o):
    module = o.__class__.__module__
    if module == None or module == str.__class__.__module__:
        return o.__class__.__name__
    else:
        return module + "." + o.__class__.__name__

class Request:
    def __init__(self, websocket):
        self.websocket = websocket

    def log(self, path, message = None):
        if message != None:
            print("{}:{} - - [{}] \"{}\": {}".format(self.websocket.remote_address[0], self.websocket.remote_address[1], datetime.datetime.now().strftime("%d/%b/%Y %H:%M:%S"), path, message))
        else:
            print("{}:{} - - [{}] \"{}\"".format(self.websocket.remote_address[0], self.websocket.remote_address[1], datetime.datetime.now().strftime("%d/%b/%Y %H:%M:%S"), path))

    def error(self, path, e):
        self.log(path, "{}: {}".format(full_class_name(e), e))

class Module:
    def __init__(self, module):
        self.module = module

    def log(self, message, submodule = None):
        if submodule != None:
            print("{}({}): {}".format(self.module, submodule, message))
        else:
            print("{}: {}".format(self.module, message))

    def error(self, e, submodule = None):
        self.log("{}: {}".format(full_class_name(e), e), submodule)

module = Module(__name__)

class Code:
    def __init__(self, number):
        self.number = number

    @classmethod
    def cleanup(cls):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM Code WHERE expiration < %s"
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (timestamp,)
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

    @classmethod
    def generate(cls, user):
        cls.cleanup()
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM Code WHERE user = %s"
            values = (user,)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "INSERT INTO Code(number, expiration, user) VALUES (%s, %s, %s)"
                number = random.randint(0, 65535)
                expiration = datetime.datetime.now() + datetime.timedelta(minutes = 5)
                timestamp = expiration.strftime("%Y-%m-%d %H:%M:%S")
                values = (number, timestamp, user)
                cursor.execute(query, values)
                connection.commit()
                return cls(number)
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    def verify(self, user):
        Code.cleanup()
        real_user = self.get_user()
        if real_user and real_user.email == user:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "DELETE FROM Code WHERE number = %s"
                values = (self.number,)
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

    def get_user(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT user FROM Code WHERE number = %s"
            values = (self.number,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return User(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_expiration(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT expiration FROM Code WHERE number = %s"
            values = (self.number,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return datetime.datetime.fromtimestamp(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

class Delivery:
    def __init__(self, id):
        self.id = id

    @classmethod
    def request(cls, sender, origin, receiver):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO Delivery(id, start_time, origin, state, sender, receiver) VALUES (%s, %s, %s, 0, %s, %s)"
            next_id = cls.next_id()
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (next_id, timestamp, origin, sender, receiver)
            try:
                cursor.execute(query, values)
                connection.commit()
                return cls(next_id)
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

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

    def response(self, destination, robot):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Delivery SET state = 1, destination = %s, robot = %s WHERE id = %s"
            values = (destination, robot, self.id)
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
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM Delivery WHERE id = %s"
            values = (self.id,)
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

    def set_path(self, path):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Delivery SET path = %s WHERE id = %s"
            values = (path, self.id)
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

    def get_sender(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT sender FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return User(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_origin(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT origin FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_robot(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT robot FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return Robot(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_receiver(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT receiver FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return User(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_destination(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT destination FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_path(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT path FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_state(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT state FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_start_time(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT start_time FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return datetime.datetime.fromtimestamp(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_finish_time(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT finish_time FROM Delivery WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return datetime.datetime.fromtimestamp(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

class Robot:
    def __init__(self, id):
        self.id = id

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
        available = cls.get_available_robots()
        if available:
            nearest = available[0]
            for robot in available:
                if distance(robot[1], position) < distance(nearest[1], position):
                    nearest = robot
            return cls(nearest[0])

    @classmethod
    def get_available_robots(cls):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT id, position FROM Robot WHERE alive = true AND state = 0"
            try:
                cursor.execute(query)
                result = cursor.fetchall()
                if result:
                    return result
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def signin(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET alive = true WHERE id = %s"
            values = (self.id,)
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

    def signout(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET alive = false WHERE id = %s"
            values = (self.id,)
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

    def set_position(self, position):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET position = %s WHERE id = %s"
            values = (position, self.id)
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

    def set_route(self, route):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET route = %s WHERE id = %s"
            values = (route, self.id)
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

    def set_state(self, state):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET state = %s WHERE id = %s"
            values = (state, self.id)
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

    def set_item(self, item):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET item = %s WHERE id = %s"
            values = (item, self.id)
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

    def set_speed(self, speed):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET speed = %s WHERE id = %s"
            values = (speed, self.id)
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

    def set_curve_radius(self, curve_radius):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET curve_radius = %s WHERE id = %s"
            values = (curve_radius, self.id)
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

    def set_left_encoder(self, left_encoder):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET left_encoder = %s WHERE id = %s"
            values = (left_encoder, self.id)
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

    def set_right_encoder(self, right_encoder):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET right_encoder = %s WHERE id = %s"
            values = (right_encoder, self.id)
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

    def set_ultrasound(self, ultrasound):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET ultrasound = %s WHERE id = %s"
            values = (ultrasound, self.id)
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

    def get_public_key(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT public_key FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return Crypto.PublicKey.RSA.importKey(bytes.fromhex(result[0]))
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_position(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT position FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_route(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT route FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_item(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT item FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_speed(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT speed FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_curve_radius(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT curve_radius FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_left_encoder(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT left_encoder FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_right_encoder(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT right_encoder FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_ultrasound(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT ultrasound FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_state(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT state FROM Robot WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

class User:
    def __init__(self, email):
        self.email = email

    @classmethod
    def check(cls, email):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT email FROM User WHERE email = %s"
            values = (email,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return True
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()
        return False

    @classmethod
    def signup(cls, email, first_name, last_name, password):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO User(email, verified, first_name, last_name) VALUES (%s, 0, %s, %s)"
            values = (email, first_name, last_name)
            try:
                cursor.execute(query, values)
                connection.commit()
                user = cls(email)
                if user.set_password(password):
                    return user
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    def delete(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM User WHERE email = %s"
            values = (self.email,)
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

    def verify(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE User SET verified = true WHERE email = %s"
            values = (self.email,)
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

    def set_email(self, email):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE User SET email = %s WHERE email = %s"
            values = (email, self.email)
            try:
                cursor.execute(query, values)
                connection.commit()
                self.email = email
                return True
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()
        return False

    def set_first_name(self, first_name):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE User SET first_name = %s WHERE email = %s"
            values = (first_name, self.email)
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

    def set_last_name(self, last_name):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE User SET last_name = %s WHERE email = %s"
            values = (last_name, self.email)
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

    def set_password(self, password):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE User SET salt = %s, hash = %s WHERE email = %s"
            salt = os.urandom(32)
            hash = hashlib.pbkdf2_hmac("sha256", password.encode("utf-8"), salt, 100000)
            values = (salt.hex(), hash.hex(), self.email)
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

    def check_password(self, password):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT salt, hash FROM User WHERE email = %s"
            values = (self.email,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    salt = bytes.fromhex(result[0])
                    hash = hashlib.pbkdf2_hmac("sha256", password.encode("utf-8"), salt, 100000)
                    if hash == bytes.fromhex(result[1]):
                        return True
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()
        return False

    def get_verified(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT verified FROM User WHERE email = %s"
            values = (self.email,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_first_name(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT first_name FROM User WHERE email = %s"
            values = (self.email,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_last_name(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT last_name FROM User WHERE email = %s"
            values = (self.email,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

class Login:
    def __init__(self, cookie):
        self.cookie = cookie

    @classmethod
    def signin(cls, user):
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

    @classmethod
    def get_cookies(cls, user):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT cookie FROM Login WHERE user = %s"
            values = (user,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchall()
                if result:
                    return result
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def signout(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM Login WHERE cookie = %s"
            values = (self.cookie,)
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

    def get_user(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT user FROM Login WHERE cookie = %s"
            values = (self.cookie,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return User(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

class QRCode:
    def __init__(self, number):
        self.number = number

    @classmethod
    def cleanup(cls):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM QRCode WHERE expiration < %s"
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (timestamp,)
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

    @classmethod
    def generate(cls, user, delivery):
        cls.cleanup()
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM QRCode WHERE delivery = %s"
            values = (delivery,)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "INSERT INTO QRCode(number, expiration, user, delivery) VALUES (%s, %s, %s, %s)"
                number = random.randint(0, 65535)
                expiration = datetime.datetime.now() + datetime.timedelta(minutes = 5)
                timestamp = expiration.strftime("%Y-%m-%d %H:%M:%S")
                values = (number, timestamp, user, delivery)
                cursor.execute(query, values)
                connection.commit()
                return cls(number)
            except Exception as e:
                module.error(e)
                connection.rollback()
            finally:
                cursor.close()

    def verify(self, user, delivery):
        QRCode.cleanup()
        real_user = self.get_user()
        real_delivery = self.get_delivery()
        if real_user and real_delivery and real_user.email == user and real_delivery.id == delivery:
            with connect.connect() as connection:
                cursor = connection.cursor(prepared = True)
                query = "DELETE FROM QRCode WHERE number = %s"
                values = (self.number,)
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

    def get_user(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT user FROM QRCode WHERE number = %s"
            values = (self.number,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return User(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_delivery(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT delivery FROM QRCode WHERE number = %s"
            values = (self.number,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return Delivery(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

    def get_expiration(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT expiration FROM QRCode WHERE number = %s"
            values = (self.number,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return datetime.datetime.fromtimestamp(result[0])
            except Exception as e:
                module.error(e)
            finally:
                cursor.close()

xy_map = [(1, 0), (3, 0), (4, 0), (6, 0), (4, 1), (5, 1), (6, 1), (5, 2),
          (6, 2), (3, 3), (4, 3), (5, 3), (6, 3), (5, 4), (6, 4), (3, 5),
          (4, 5), (5, 5), (6, 5), (1, 7), (3, 7), (4, 7), (6, 7), (2, 3),
          (1, 2), (0, 2), (0, 6), (1, 6), (2, 5)]

def distance(p1, p2):
    x1, y1 = xy_map[p1]
    x2, y2 = xy_map[p2]
    return math.sqrt((x1 - x2)**2 + (y1 - y2)**2)
