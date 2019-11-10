import mysql.connector
import objects

module = objects.Module(__name__)

class connect:
    def __init__(self):
        self.connection = mysql.connector.connect(host = "dlvr.cvxnw3a2asye.sa-east-1.rds.amazonaws.com", database = "dlvr", user = "admin", password = "dlvr_oi3_2019_2")

    def __enter__(self):
        if self.connection.is_connected():
            return self.connection

    def __exit__(self, type, value, traceback):
        if isinstance(value, Exception):
            module.error(value)

        try:
            self.connection.close()
        finally:
            return True
