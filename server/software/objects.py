import connect
import mysql.connector
import random
import datetime
import Crypto.PublicKey.RSA
import os
import hashlib

class Code:
    def __init__(self, number):
        self.number = number

    @classmethod
    def generate(cls, email):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM Code WHERE expiration < %s"
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (timestamp,)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "DELETE FROM Code WHERE user = %s"
                values = (email,)
                cursor.execute(query, values)
                connection.commit()
                query = "INSERT INTO Code(number, expiration, user) VALUES (%s, %s, %s)"
                number = random.randint(0, 65535)
                expiration = datetime.datetime.now() + datetime.timedelta(minutes = 5)
                timestamp = expiration.strftime("%Y-%m-%d %H:%M:%S")
                values = (number, timestamp, email)
                cursor.execute(query, values)
                connection.commit()
                return cls(number)
            except mysql.connector.Error as e:
                print(e)
                connection.rollback()
            finally:
                cursor.close()

    def verify(self, email):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM Code WHERE expiration < %s"
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (timestamp,)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "SELECT user FROM Code WHERE number = %s"
                values = (self.number,)
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result and result[0] == email:
                    query = "DELETE FROM Code WHERE number = %s"
                    cursor.execute(query, values)
                    connection.commit()
                    return True
            except mysql.connector.Error as e:
                print(e)
                connection.rollback()
            finally:
                cursor.close()
        return False

class Delivery:
    def __init__(self, id):
        self.id = id

    @classmethod
    def request(cls, sender, origin, receiver):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO Delivery(start_time, origin, state, sender, receiver) VALUES (%s, %s, 0, %s, %s)"
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (timestamp, origin, sender, receiver)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "SELECT id FROM Delivery WHERE start_time = %s AND origin = %s AND state = 0 AND sender = %s AND receiver = %s"
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return cls(result[0])
            except mysql.connector.Error as e:
                print(e)
                connection.rollback()
            finally:
                cursor.close()

    def response(self, destination, robot):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Delivery SET state = 1, destination = %s, robot = %s WHERE id = %s"
            values = (destination, robot, self.id)
            try:
                cursor.execute(query, values)
                connection.commit()
                return True
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
                    return result[0]
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
                    return result[0]
            except mysql.connector.Error as e:
                print(e)
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
                    return result[0]
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
            finally:
                cursor.close()

class Robot:
    def __init__(self, id):
        self.id = id

    def signin(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "UPDATE Robot SET alive = true WHERE id = %s"
            values = (self.id,)
            try:
                cursor.execute(query, values)
                connection.commit()
                return True
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
            finally:
                cursor.close()
        return False

    @classmethod
    def signup(cls, email, first_name, last_name, password):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO User(email, verified, first_name, last_name, salt, hash) VALUES (%s, 0, %s, %s, %s, %s)"
            salt = os.urandom(32)
            hash = hashlib.pbkdf2_hmac("sha256", password.encode("utf-8"), salt, 100000)
            values = (email, first_name, last_name, salt.hex(), hash.hex())
            try:
                cursor.execute(query, values)
                connection.commit()
                return cls(email)
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
            finally:
                cursor.close()

class Login:
    def __init__(self, cookie):
        self.cookie = cookie

    @classmethod
    def signin(cls, email):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "INSERT INTO Login(cookie, email) VALUES (%s, %s)"
            cookie = os.urandom(32).hex()
            values = (cookie, email)
            try:
                cursor.execute(query, values)
                connection.commit()
                return cls(cookie)
            except mysql.connector.Error as e:
                print(e)
                connection.rollback()
            finally:
                cursor.close()

    @classmethod
    def get_cookies(cls, email):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT cookie FROM Login WHERE email = %s"
            values = (email,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchall()
                if result:
                    return result
            except mysql.connector.Error as e:
                print(e)
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
            except mysql.connector.Error as e:
                print(e)
                connection.rollback()
            finally:
                cursor.close()
        return False

    def get_email(self):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "SELECT email FROM Login WHERE cookie = %s"
            values = (self.cookie,)
            try:
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result:
                    return result[0]
            except mysql.connector.Error as e:
                print(e)
            finally:
                cursor.close()

class QRCode:
    def __init__(self, number):
        self.number = number

    @classmethod
    def generate(cls, user, delivery):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM QRCode WHERE expiration < %s"
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (timestamp,)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "DELETE FROM QRCode WHERE delivery = %s"
                values = (delivery,)
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
            except mysql.connector.Error as e:
                print(e)
                connection.rollback()
            finally:
                cursor.close()

    def verify(self, user, delivery):
        with connect.connect() as connection:
            cursor = connection.cursor(prepared = True)
            query = "DELETE FROM QRCode WHERE expiration < %s"
            timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            values = (timestamp,)
            try:
                cursor.execute(query, values)
                connection.commit()
                query = "SELECT user, delivery FROM QRCode WHERE number = %s"
                values = (self.number,)
                cursor.execute(query, values)
                result = cursor.fetchone()
                if result and result[0] == user and result[1] == delivery:
                    query = "DELETE FROM QRCode WHERE number = %s"
                    cursor.execute(query, values)
                    connection.commit()
                    return True
            except mysql.connector.Error as e:
                print(e)
                connection.rollback()
            finally:
                cursor.close()
        return False
