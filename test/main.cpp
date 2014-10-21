#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <sstream>
#include <iomanip>

#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include <RTPS.h>

using namespace rtps;

int window_width = 640*2;
int window_height = 480*2;
int glutWindowHandle = 0;


#define DTR 0.0174532925

struct camera
{
    GLdouble leftfrustum;
    GLdouble rightfrustum;
    GLdouble bottomfrustum;
    GLdouble topfrustum;
    GLfloat modeltranslation;
} leftCam, rightCam;

bool stereo_enabled = false;
bool render_movie = false;

unsigned int frame_counter = 0;
float depthZ = -10.0;                                      //depth of the object drawing
double fovy = 65.;                                          //field of view in y-axis
double aspect = double(window_width)/double(window_height);  //screen aspect ratio
double nearZ = 0.3;                                        //near clipping plane
double farZ = 100.0;                                        //far clipping plane
double screenZ = 10.0;                                     //screen projection plane
double IOD = 0.5;                                          //intraocular distance

float translate_x = -2.00f;
float translate_y = -2.70f;
float translate_z = 3.50f;

float arm_translate_x = 0.;
float arm_translate_y = 0.;
float arm_translate_z = 0.;
float arm_velocity_x = 0.;
float arm_velocity_y = 0.;
float arm_velocity_z = 0.;

// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
std::vector<Triangle> triangles;

void init_gl();
void render_stereo();
void setFrustum();

void appKeyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void appRender();
void appDestroy();

void appMouse(int button, int state, int x, int y);
void appMotion(int x, int y);
void resizeWindow(int w, int h);

void timerCB(int ms);

void drawString(const char *str, int x, int y, float color[4], void *font);
void showFPS(float fps, std::string *report);
void draw_collision_boxes();

void *font = GLUT_BITMAP_8_BY_13;

rtps::RTPS* ps;

#define NUM_PARTICLES 100000

#define DT .003f

float4 color = float4(.0, 0.7, 0.0, 1.);
int hindex; 

//================
#include "materials_lights.h"

//----------------------------------------------------------------------
float rand_float(float mn, float mx)
{
    float r = rand() / (float) RAND_MAX;
    return mn + (mx-mn)*r;
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
int main(int argc, char** argv)
{
    //initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - window_width/2, 
                            glutGet(GLUT_SCREEN_HEIGHT)/2 - window_height/2);

    int max_num = rtps::nlpo2(NUM_PARTICLES);
    std::stringstream ss;
    ss << "Real-Time Particle System: " << max_num << std::ends;
    glutWindowHandle = glutCreateWindow(ss.str().c_str());

    glutDisplayFunc(appRender); //main rendering function
    glutTimerFunc(30, timerCB, 30); //determin a minimum time between frames
    glutKeyboardFunc(appKeyboard);
    glutMouseFunc(appMouse);
    glutMotionFunc(appMotion);
    glutReshapeFunc(resizeWindow);

    define_lights_and_materials();

    glewInit();
    GLboolean bGLEW = glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"); 
    printf("GLEW supported?: %d\n", bGLEW);

    rtps::Domain* grid = new Domain(float4(0,0,0,0), float4(5, 5, 5, 0));

	rtps::RTPSettings* settings = new rtps::RTPSettings(rtps::RTPSettings::SPH, NUM_PARTICLES, DT, grid);
    settings->SetSetting("rtps_path", "./bin");

    settings->setRenderType(RTPSettings::RENDER);
    settings->setRadiusScale(0.4);
    settings->setBlurScale(1.0);
    settings->setUseGLSL(1);

    settings->SetSetting("sub_intervals", 1);
    settings->SetSetting("render_texture", "firejet_blast.png");
    settings->SetSetting("render_frag_shader", "sprite_tex_frag.glsl");
    settings->SetSetting("render_use_alpha", true);
    settings->SetSetting("render_alpha_function", "add");
    settings->SetSetting("lt_increment", -.00);
    settings->SetSetting("lt_cl", "lifetime.cl");

    ps = new rtps::RTPS(settings);

    ps->settings->SetSetting("Gravity", -9.8f); // -9.8 m/sec^2
    ps->settings->SetSetting("Gas Constant", 1.0f);
    ps->settings->SetSetting("Viscosity", .001f);
    ps->settings->SetSetting("Velocity Limit", 600.0f);
    ps->settings->SetSetting("XSPH Factor", .15f);
    ps->settings->SetSetting("Friction Kinetic", 0.0f);
    ps->settings->SetSetting("Friction Static", 0.0f);
    ps->settings->SetSetting("Boundary Stiffness", 20000.0f);
    ps->settings->SetSetting("Boundary Dampening", 256.0f);
    init_gl();
    printf("about to start main loop\n");
    glutMainLoop();
    return 0;
}



void init_gl()
{
    glViewport(0, 0, window_width, window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.2, .2, .6, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ps->system->getRenderer()->setWindowDimensions(window_width,window_height);
    return;
}

void appKeyboard(unsigned char key, int x, int y)
{
    int nn;
    float4 min;
    float4 max;
    switch (key)
    {
        case 'e': //dam break
            {
                nn = 16384;
                min = float4(.1, .1, .1, 1.0f);
                max = float4(3.9, 3.9, 3.9, 1.0f);
                ps->system->addBox(nn, min, max, false,color);
                return;
            }
        case 'p': //print timers
            ps->system->printTimers();
            return;
        case '\033': // escape quits
        case '\015': // Enter quits    
        case 'Q':    // Q quits
        case 'q':    // q (or escape) quits
            appDestroy();
            return;
        case 'b':
            printf("deleting willy nilly\n");
            ps->system->testDelete();
           return;
        case 'h':
        {
            //spray hose
            printf("about to make hose\n");
            float4 center(1., 2., 2., 1.);
            float4 velocity(2., .5, 2., 0);
            hindex = ps->system->addHose(5000, center, velocity, 4, color);
            return;
		}
        case 'H':
        {
            //spray hose
            printf("about to move hose\n");
            float4 center(.1, 2., 1., 1.);
            float4 velocity(2., -.5, -1., 0);
            ps->system->updateHose(hindex, center, velocity, 4, color);
            return;
		}
        case 'n':
            render_movie=!render_movie;
            break;
        case '`':
            stereo_enabled = !stereo_enabled;
            break;
        case 't': //place a cube for collision
            {
                nn = 512;
                float cw = .25;
                float4 cen = float4(cw, cw, cw-.1, 1.0f);
                make_cube(triangles, cen, cw);
                cen = float4(1+cw, 1+cw, cw-.1, 1.0f);
                make_cube(triangles, cen, cw);
                cen = float4(1+3*cw, 1+3*cw, cw-.1, 1.0f);
                make_cube(triangles, cen, cw);
                cen = float4(3.5, 3.5, cw-.1, 1.0f);
                make_cube(triangles, cen, cw);

                cen = float4(1.5, 1.5, cw-.1, 1.0f);
                make_cube(triangles, cen, 1.);

                ps->system->loadTriangles(triangles);
                return;
            }
        case 'r': //drop a rectangle
            {
                min = float4(2.0, 2.0, 3.2, 1.0f);
                max = float4(3.0, 3.0, 4.0, 1.0f);
                ps->system->addBox(nn, min, max, false, color);
                return;
            }

        case 'o':
            ps->system->getRenderer()->writeBuffersToDisk();
            return;
        case 'c':
            ps->system->getRenderer()->setDepthSmoothing(Render::NO_SHADER);
            return;
        case 'C':
            ps->system->getRenderer()->setDepthSmoothing(Render::BILATERAL_GAUSSIAN_SHADER);
            return;
        case 'w':
            translate_z -= 0.1;
            break;
        case 'a':
            translate_x += 0.1;
            break;
        case 's':
            translate_z += 0.1;
            break;
        case 'd':
            translate_x -= 0.1;
            break;

        case 'W':
            arm_translate_z -= 0.1;
            arm_velocity_z  += 0.1;
            break;
        case 'A':
			// move hand at constant velocity in x
            arm_translate_x += 0.1;
            arm_velocity_x  += 0.1;
            break;
        case 'S':
            arm_translate_z += 0.1;
            arm_velocity_z  += 0.1;
            break;
        case 'D':
            arm_translate_x -= 0.1;
            arm_velocity_x  -= 0.1;
            break;
        case 'Z':
            arm_translate_y += 0.1;
            arm_velocity_y  += 0.1;
            break;
        case 'X':
            arm_translate_y -= 0.1;
            arm_velocity_y  -= 0.1;
            break;
        case 'z':
            translate_y += 0.1;
            break;
        case 'x':
            translate_y -= 0.1;
            break;
        default:
            return;
    }
    glutPostRedisplay();
}

void timerCB(int ms)
{
    glutTimerFunc(ms, timerCB, ms);
    ps->update();
    glutPostRedisplay();
}

void appRender()
{
    glEnable(GL_DEPTH_TEST);
    if (stereo_enabled)
    {
        render_stereo();
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fovy, aspect, nearZ, farZ);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-90, 1.0, 0.0, 0.0);
        glRotatef(rotate_x, 1.0, 0.0, 0.0);
        glRotatef(rotate_y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
        glTranslatef(translate_x, translate_z, translate_y);
        ps->render();
        draw_collision_boxes();
        
    }

    if(render_movie)
    {
        frame_counter++;
    }
    glutSwapBuffers();
}

void appDestroy()
{
    delete ps;
    if (glutWindowHandle)glutDestroyWindow(glutWindowHandle);
    printf("about to exit!\n");
    exit(0);
}

void appMouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        mouse_buttons |= 1<<button;
    }
    else if (state == GLUT_UP)
    {
        mouse_buttons = 0;
    }
    mouse_old_x = x;
    mouse_old_y = y;
}

void appMotion(int x, int y)
{
    float dx, dy;
    dx = x - mouse_old_x;
    dy = y - mouse_old_y;

    if (mouse_buttons & 1)
    {
        rotate_x += dy * 0.2;
        rotate_y += dx * 0.2;
    }
    else if (mouse_buttons & 4)
    {
        translate_z -= dy * 0.1;
    }

    mouse_old_x = x;
    mouse_old_y = y;
    glutPostRedisplay();
}


///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_LIGHTING);
    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
// display frame rates
///////////////////////////////////////////////////////////////////////////////
void showFPS(float fps, std::string* report)
{
    static std::stringstream ss;

    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, 400, 0, 300);         // set to orthogonal projection

    float color[4] = {1, 1, 0, 1};

    // update fps every second
    ss.str("");
    ss << std::fixed << std::setprecision(1);
    ss << fps << " FPS" << std::ends; // update fps string
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
    drawString(ss.str().c_str(), 15, 286, color, font);
    drawString(report[0].c_str(), 15, 273, color, font);
    drawString(report[1].c_str(), 15, 260, color, font);

    glPopMatrix();                      // restore to previous projection matrix
    glMatrixMode(GL_MODELVIEW);         // switch to modelview matrix
    glPopMatrix();                      // restore to previous modelview matrix
}
//----------------------------------------------------------------------
void resizeWindow(int w, int h)
{
    if (h==0)
    {
        h=1;
    }
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ps->system->getRenderer()->setWindowDimensions(w,h);
    window_width = w;
    window_height = h;
    setFrustum();
    glutPostRedisplay();
}

void render_stereo()
{

    glDrawBuffer(GL_BACK_LEFT);                              //draw into back left buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();                                        //reset projection matrix
    glFrustum(leftCam.leftfrustum, leftCam.rightfrustum,     //set left view frustum
              leftCam.bottomfrustum, leftCam.topfrustum,
              nearZ, farZ);
    glTranslatef(leftCam.modeltranslation, 0.0, 0.0);        //translate to cancel parallax
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    {
        glRotatef(-90, 1.0, 0.0, 0.0);
        glRotatef(rotate_x, 1.0, 0.0, 0.0);
        glRotatef(rotate_y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
        glTranslatef(translate_x, translate_z, translate_y);
        ps->render();
        draw_collision_boxes();
    }
    glPopMatrix();
    glDrawBuffer(GL_BACK_RIGHT);                             //draw into back right buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();                                        //reset projection matrix
    glFrustum(rightCam.leftfrustum, rightCam.rightfrustum,   //set left view frustum
              rightCam.bottomfrustum, rightCam.topfrustum,
              nearZ, farZ);
    glTranslatef(rightCam.modeltranslation, 0.0, 0.0);       //translate to cancel parallax
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    {
        glRotatef(-90, 1.0, 0.0, 0.0);
        glRotatef(rotate_x, 1.0, 0.0, 0.0);
        glRotatef(rotate_y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
        glTranslatef(translate_x, translate_z, translate_y);
        ps->render();
        draw_collision_boxes();
    }
    glPopMatrix();
}


void setFrustum(void)
{
    double top = nearZ*tan(DTR*fovy/2);                    //sets top of frustum based on fovy and near clipping plane
    double right = aspect*top;                             //sets right of frustum based on aspect ratio
    double frustumshift = (IOD/2)*nearZ/screenZ;

    leftCam.topfrustum = top;
    leftCam.bottomfrustum = -top;
    leftCam.leftfrustum = -right + frustumshift;
    leftCam.rightfrustum = right + frustumshift;
    leftCam.modeltranslation = IOD/2;

    rightCam.topfrustum = top;
    rightCam.bottomfrustum = -top;
    rightCam.leftfrustum = -right - frustumshift;
    rightCam.rightfrustum = right - frustumshift;
    rightCam.modeltranslation = -IOD/2;
}

void draw_collision_boxes()
{
    glColor4f(0,0,1,.5);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLES);
    for (int i=0; i < triangles.size(); i++)
    {
        Triangle& tria = triangles[i];
        glNormal3fv(&tria.normal.x);
        glVertex3f(tria.verts[0].x, tria.verts[0].y, tria.verts[0].z);
        glVertex3f(tria.verts[1].x, tria.verts[1].y, tria.verts[1].z);
        glVertex3f(tria.verts[2].x, tria.verts[2].y, tria.verts[2].z);
    }
    glEnd();
    glDisable(GL_BLEND);
}
