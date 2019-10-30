import mysql.connector

def connect():
    try:
        connection = mysql.connector.connect(host = "dlvr.cvxnw3a2asye.sa-east-1.rds.amazonaws.com", database = "dlvr", user = "admin", password = "dlvr_oi3_2019_2")
        if connection.is_connected():
            return connection
    except mysql.connector.Error as e:
        print(e)
