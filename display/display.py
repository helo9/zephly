import pygame as pg
from pygame.locals import *

from OpenGL.GL import *
from OpenGL.GLU import *

import serial
import socket
import json
import quaternionic

from math import acos, sqrt, degrees

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

def main():

    ser = SerialReader()

    pg.init()

    pg.display.set_mode(graphic_options.window_size, DOUBLEBUF | OPENGL)
    pg.display.set_caption("orientation")

    q = [1.0, 0.0, 0.0, 0.0]

    while True:
        event = pg.event.poll()
 
        
        if should_exit(event):
            break

        res = ser.read_all()
        
        if res:
            q = res[0]
            send(res[1])            

        norm = sqrt(q[0]**2 + q[1]**2 + q[2]**2 + q[3]**2)

        q = [qp/norm for qp in q]

        draw_cube(to_axis_angle(q))

        pg.display.flip()
        pg.time.wait(1)

    ser.close()


def to_axis_angle(q):

    qw, qx, qy, qz = q
    angle = degrees(2 * acos(qw))
    s = sqrt(1 - qw ** 2)

    if (abs(s) < 1e-9):
        angle = 0.0
        x = 0.0
        y = 0.0
        z = 0.0
    else:
        x = qx / s
        y = qy / s
        z = qz / s
    return angle, x, y, z


def send(rot_speed):
    print(rot_speed)
    data = {
        "rotation_speed": {
            "x": rot_speed[0],
            "y": rot_speed[1],
            "z": rot_speed[2]
        }
    }
    sock.sendto( json.dumps(data).encode(), ("127.0.0.1", 9870) )

def should_exit(event):
    return event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE)

class SerialReader:
    def __init__(self, port="/dev/ttyACM0", baudrate=115200):
        self.ser = serial.Serial(port, baudrate=baudrate)

    def read_quaternion(self):

        line = self.ser.readline().decode()

        try:
            quaternion_list = [float(val) for val in line.split(" ")[:4]]
        except ValueError:
            print('value error')
            return None

        if len(quaternion_list) == 4:
            print(quaternion_list)
            return quaternion_list
        else:
            print('wrong len..', quaternion_list)
            return None

    def read_all(self):

        line = self.ser.readline().decode()

        try:
            quaternion_list = [float(val) for val in line.split(" ")[:4]]
        except ValueError:
            print('value error')
            return None


        try:
            rot_vel = [float(val) for val in line.split(" ")[5:8]]
        except ValueError:
            print('bvalue error', line.split(" ")[5:])
            return None

        if len(quaternion_list) == 4 and len(rot_vel) == 3:
            return quaternion_list, rot_vel
        else:
            print('wrong len..', quaternion_list)
            return None

    def close(self):
        self.ser.close()


class graphic_options:
    window_size = (1920, 1080)

    verticies = (
        (1, -1, -1),
        (1, 1, -1),
        (-1, 1, -1),
        (-1, -1, -1),
        (1, -1, 1),
        (1, 1, 1),
        (-1, -1, 1),
        (-1, 1, 1),
    )

    edges = (
        (0, 1),
        (0, 3),
        (0, 4),
        (2, 1),
        (2, 3),
        (2, 7),
        (6, 3),
        (6, 4),
        (6, 7),
        (5, 1),
        (5, 4),
        (5, 7),
    )


def draw_cube(rot_anglevector=(15.0, 0.0, 1.0, 0.0)):
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()

    gluPerspective(
        45,
        (float(graphic_options.window_size[0]) / graphic_options.window_size[1]),
        0.1,
        15.0,
    )

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    glTranslatef(0.0, 0.0, -12.01)
    glRotatef(*rot_anglevector)

    glBegin(GL_LINES)
    for edge in graphic_options.edges:
        for vertex in edge:
            glVertex3fv(graphic_options.verticies[vertex])
    glEnd()


def do_transform():
    pass


if __name__ == "__main__":
    main()
