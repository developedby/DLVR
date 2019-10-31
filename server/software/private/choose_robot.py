import connect
import mysql.connector

def get_available_robots():
    with connect.connect() as connection:
        cursor = connection.cursor(prepared = True)
        query = "SELECT id, position, state FROM Robot WHERE alive = true"
        try:
            cursor.execute(query)
            result = cursor.fetchall()
            return result
        except mysql.connector.Error as e:
            print(e)
        finally:
            cursor.close()

def nearest_robot(robots, position):
    min_dist = 424242
    rid = -1
    for robot in robots:
        dist = (robot[1] - position)**2
        if dist < min_dist:
            min_dist = dist
            rid = robot[0]
    return rid

def choose(position):
    robots = get_available_robots()
    rid = nearest_robot(robots, position)
    return rid
