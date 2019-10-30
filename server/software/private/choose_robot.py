#!/usr/bin/env python3
import mysql.connector
import connect
import mysql.connector


__QUERY = "SELECT id, position, state FROM Robot"

def get_available_robots():
    global __QUERY
    connection = connect.connect()
    cursor = connection.cursor(prepared = True)
    cursor.execute(__QUERY, ())
    result = cursor.fetchall()
    return result

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

