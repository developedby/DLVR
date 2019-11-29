#!/usr/bin/env python3

import asyncio
import objects

async def main():
    robot = objects.Robot(0)
    robot.state = 0
    robot.position = 1
    robot.orientation = objects.Direction.WEST
    robot.route = []
    robot.alive = False
    for delivery in robot.deliveries:
        if delivery.state < 5:
            delivery.delete()
    user1 = objects.User("subject@example.com")
    for login in user1.logins:
        login.delete()
    user2 = objects.User("subject2@example.com")
    for login in user2.logins:
        login.delete()

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
