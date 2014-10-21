#basic glut setup learned from here:
#http://www.java2s.com/Open-Source/Python/Game-2D-3D/PyOpenGL/PyOpenGL-Demo-3.0.1b1/PyOpenGL-Demo/NeHe/lesson2.py.htm

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys

#helper modules
import glutil
from vector import Vec

#OpenCL code
import boids
#functions for initial values of particles
import initialize

#number of particles
max_num = 200000
#time step for integration
dt = .01

class window(object):
    def __init__(self, *args, **kwargs):
        #mouse handling for transforming scene
        self.mouse_down = False
        self.mouse_old = Vec([0., 0.])
        self.rotate = Vec([0., 0., 0.])
        self.translate = Vec([0., 0., 0.])
        self.initrans = Vec([0., 0., -200.])

        self.width = 640
        self.height = 480

        glutInit(sys.argv)
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
        glutInitWindowSize(self.width, self.height)
        glutInitWindowPosition(0, 0)
        self.win = glutCreateWindow("Part 2: Python")

        #gets called by GLUT every frame
        glutDisplayFunc(self.draw)

        #handle user input
        glutKeyboardFunc(self.on_key)
        glutMouseFunc(self.on_click)
        glutMotionFunc(self.on_mouse_motion)
        
        #this will call draw every 30 ms
        glutTimerFunc(30, self.timer, 30)

        self.dim = 300
       
        #setup OpenGL scene
        self.glinit()


        #set up initial conditions
        (pos_vbo, col_vbo, vel, acc, num) = initialize.boids(max_num)
        #create our OpenCL instance
        self.boids = boids.Boids(num, dt, self.dim)
        self.boids.loadData(pos_vbo, col_vbo, vel, acc)

        glutMainLoop()
        
    def draw(self):
        """Render the particles"""        
        #update or particle positions by calling the OpenCL kernel
        #for i in xrange(0, 10): #10 updates per frame
        #    self.boids.execute() 
        self.boids.rules()
        self.boids.euler()
        glFlush()

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

        #handle mouse transformations
        """
        glTranslatef(self.initrans.x, self.initrans.y, self.initrans.z)
        glRotatef(self.rotate.x, 1, 0, 0)
        glRotatef(self.rotate.y, 0, 1, 0) #we switched around the axis so make this rotate_z
        glTranslatef(self.translate.x, self.translate.y, self.translate.z)
        """
        
        #render the particles
        self.boids.render()

        #draw the x, y and z axis as lines
        glutil.draw_axes()

        glutSwapBuffers()


    def glinit(self):
        glViewport(0, 0, self.width, self.height)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(-self.dim, self.dim, -self.dim, self.dim, -1, 1);
        #gluPerspective(60., self.width / float(self.height), .1, 1000.)
        glMatrixMode(GL_MODELVIEW)


    ###GL CALLBACKS
    def timer(self, t):
        glutTimerFunc(t, self.timer, t)
        glutPostRedisplay()

    def on_key(self, *args):
        ESCAPE = '\033'
        if args[0] == ESCAPE or args[0] == 'q':
            sys.exit()

    def on_click(self, button, state, x, y):
        if state == GLUT_DOWN:
            self.mouse_down = True
            self.button = button
        else:
            self.mouse_down = False
        self.mouse_old.x = x
        self.mouse_old.y = y

    
    def on_mouse_motion(self, x, y):
        dx = x - self.mouse_old.x
        dy = y - self.mouse_old.y
        if self.mouse_down and self.button == 0: #left button
            self.rotate.x += dy * .2
            self.rotate.y += dx * .2
        elif self.mouse_down and self.button == 1: #right button
            self.translate.z -= dy * .01 
        self.mouse_old.x = x
        self.mouse_old.y = y
    ###END GL CALLBACKS





if __name__ == "__main__":
    p2 = window()



