import connect
import mysql.connector
import hashlib
import http.cookies
import os

def signin(email, password):
    connection = connect.connect()
    cursor = connection.cursor(prepared = True)
    query = "SELECT verified, salt, hash FROM User WHERE email = %s"
    values = (email,)
    try:
        cursor.execute(query, values)
        result = cursor.fetchone()
        if result and result[0]:
            salt = bytes.fromhex(result[1])
            hash = hashlib.pbkdf2_hmac("sha256", password.encode("utf-8"), salt, 100000)
            if hash == bytes.fromhex(result[2]):
                cookie = http.cookies.SimpleCookie()
                cookie["token"] = os.urandom(32).hex()
                query = "INSERT INTO Login(cookie, email) VALUES (%s, %s)"
                values = (cookie["token"].value, email,)
                try:
                    cursor.execute(query, values)
                    connection.commit()
                    cursor.close()
                    connection.close()
                    return cookie
                except mysql.connector.Error:
                    connection.rollback()
                    cursor.close()
                    connection.close()
            else:
                cursor.close()
                connection.close()
        else:
            cursor.close()
            connection.close()
    except mysql.connector.Error:
        cursor.close()
        connection.close()


def user_email(token):
    connection = connect.connect()
    cursor = connection.cursor(prepared = True)
    query = "SELECT email FROM Login WHERE cookie = %s"
    values = (token,)
    try:
        cursor.execute(query, values)
        result = cursor.fetchone()
        if result:
            return result[0]
    except mysql.connector.Error:
        pass
    cursor.close()
    connection.close()

def signout(token):
    connection = connect.connect()
    cursor = connection.cursor(prepared = True)
    query = "DELETE FROM Login WHERE cookie = %s"
    values = (token,)
    try:
        cursor.execute(query, values)
        connection.commit()
        cursor.close()
        connection.close()
        return True
    except mysql.connector.Error:
        connection.rollback()
        cursor.close()
        connection.close()
        return False
