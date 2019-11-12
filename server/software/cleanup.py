#!/usr/bin/env python3

import objects

robot = objects.Robot(0)
robot.state = 0
robot.position = 0
for delivery in robot.deliveries:
    if delivery.state < 5:
        delivery.delete()
robot.alive = False
user1 = objects.User("subject@example.com")
for login in user1.logins:
    login.delete()
user2 = objects.User("subject2@example.com")
for login in user2.logins:
    login.delete()
