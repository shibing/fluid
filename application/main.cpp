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


unsigned int frame_counter = 0;
float depthZ = -10.0;                                      //depth of the object drawing
double fovy = 65.;                                          //field of view in y-axis
double aspect = double(window_width)/double(window_height);  //screen aspect ratio
double nearZ = 0.3;                                        //near clipping plane
double farZ = 100.0;                                        //far clipping plane
double screenZ = 10.0;                                     //screen projection plane
double IOD = 0.5;                                          //intraocular distance

float translate_x = .00f;
float translate_y = .00f;
float translate_z = -10.0f;
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

void appKeyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void appRender();
void appDestroy();

void appMouse(int button, int state, int x, int y);
void appMotion(int x, int y);
void resizeWindow(int w, int h);

void timerCB(int ms);

static bool show_info = true;
void showInfo();
static bool show_help = true;
void showHelp();
void drawString(const char *str, int x, int y, float color[4], void *font);
void draw_collision_boxes();


rtps::RTPS* ps;

#define NUM_PARTICLES 100000

#define DT .003f

float4 color = float4(.0, 0.7, 0.0, 1.);
int hindex; 

static const int FRAME_COUNT = 5 ;
static float per_frame_times[FRAME_COUNT];
static float fps = 0;

#define clear_frame_time() \
    do { \
        for(int i = 0; i < FRAME_COUNT; ++i) \
            per_frame_times[i] = 0; \
    }while(0)

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
    ss << "SPH Fluid" << std::ends;
    glutWindowHandle = glutCreateWindow(ss.str().c_str());

    glutDisplayFunc(appRender); //main rendering function
    glutTimerFunc(30, timerCB, 30); //determin a minimum time between frames
    glutKeyboardFunc(appKeyboard);
    glutMouseFunc(appMouse);
    glutMotionFunc(appMotion);
    glutReshapeFunc(resizeWindow);

    define_lights_and_materials();


    rtps::Domain* grid = new Domain(float4(-2.5,-2.5,-2.5,0), float4(2.5, 2.5, 2.5, 0));

    glewInit();
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

    init_gl();
    clear_frame_time();
    glutMainLoop();
    return 0;
}



void init_gl()
{
    glViewport(0, 0, window_width, window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.0, .0, .0, 1.0);
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
        case 'i':
            show_info = !show_info;
            break;
        case 'H':
            show_help = !show_help;
            break;
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
            float4 center(1., 2., 2., 1.);
            float4 velocity(2., .5, 2., 0);
            hindex = ps->system->addHose(5000, center, velocity, 4, color);
            return;
		}
        case 'u':
        {
            //spray hose
            float4 center(.1, 2., 1., 1.);
            float4 velocity(2., -.5, -1., 0);
            ps->system->updateHose(hindex, center, velocity, 4, color);
            return;
		}
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
        case 'r': 
            {
                nn = 2048;
                min = float4(-0.5, -0.5, 0.7, 1.0f);
                max = float4( 0.5, 0.5, 1.5, 1.0f);
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
    static int i = 0;
    static int count = 0;
    struct timespec prev_time;
    struct timespec post_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &prev_time);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, window_width, window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, aspect, nearZ, farZ);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(translate_x, translate_y, translate_z);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 1.0, 0.0); 
    ps->render();
    draw_collision_boxes();
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &post_time);

    float duration = (post_time.tv_nsec - prev_time.tv_nsec + post_time.tv_sec * 1e9 - prev_time.tv_sec * 1e9) * 1e-9;
    per_frame_times[i++] = duration;
    i %= FRAME_COUNT;
    ++count;
    if(count >= FRAME_COUNT)
        count = FRAME_COUNT;
    float all_time = 0;
    for(int j = 0; j < count; ++j)
        all_time += per_frame_times[j];
    fps = 1.0f / (all_time / count);
    if(show_info)
        showInfo();
    if(show_help)
        showHelp();
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
return false;

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

void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); 
    glDisable(GL_LIGHTING);     
    glDisable(GL_DEPTH_TEST);

    glColor4fv(color);         
    glRasterPos2i(x, y);      

    while (*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



template<typename T>
void drawValue(const string& key, const T& value, int x, int y, void *font)
{
    float color[4] = {1, 1, 0, 1};
    static std::stringstream ss;
    ss.str("");
    ss << key << ": " << value << std::ends; 
    drawString(ss.str().c_str(), x, y, color, font);
}

void showInfo()
{ 
    void *font = GLUT_BITMAP_8_BY_13;
    glPushMatrix();                    
    glLoadIdentity();                   
    glMatrixMode(GL_PROJECTION);        
    glPushMatrix();                     
    glLoadIdentity();                  
    gluOrtho2D(0, window_width, 0, window_height);       
    glTranslatef(0, window_height, 0);
    glScalef(1.0f, -1.0f, 1.0f);
    int start_pos_x = 15;
    int start_pos_y = 30; 
    drawValue("FPS", fps, start_pos_x, start_pos_y,font);
    drawValue("Maximum Number of Particles", ps->settings->GetSettingAs<int>("Maximum Number of Particles"), start_pos_x, start_pos_y + 20, font);

    drawValue("Number of Particles", ps->settings->GetSettingAs<int>("Number of Particles"), start_pos_x, start_pos_y + 40, font); 

    drawValue("Mass", ps->settings->GetSettingAs<float>("Mass"), start_pos_x, start_pos_y + 60, font);
    drawValue("Smoothing Distance", ps->settings->GetSettingAs<float>("Smoothing Distance"), start_pos_x, start_pos_y + 80, font);

    drawValue("Gas Constant(K)", ps->settings->GetSettingAs<int>("Gas Constant"), start_pos_x, start_pos_y + 100, font);

    drawValue("Gravity", ps->settings->GetSettingAs<float>("Gravity"), start_pos_x, start_pos_y + 120, font);
    drawValue("Viscosity", ps->settings->GetSettingAs<float>("Viscosity"), start_pos_x, start_pos_y + 140, font);
    drawValue("Velocity Limit", ps->settings->GetSettingAs<float>("Velocity Limit"), start_pos_x, start_pos_y + 160,font);
    
    glPopMatrix();                      
    glMatrixMode(GL_MODELVIEW);         
    glPopMatrix();                      
}

void showHelp()
{
    void *font = GLUT_BITMAP_8_BY_13;
    float color[4] = {1, 1, 0, 1};
    glPushMatrix();                    
    glLoadIdentity();                   
    glMatrixMode(GL_PROJECTION);        
    glPushMatrix();                     
    glLoadIdentity();                  
    gluOrtho2D(0, window_width, 0, window_height);       
    glTranslatef(window_width, window_height, 0);
    glScalef(-1.0f, -1.0f, 1.0f);
    int start_pos_x = 260;
    int start_pos_y = 30; 
    drawString("Help Infomation:", start_pos_x, start_pos_y, color, font);
    drawString("H: Show Help Information", start_pos_x, start_pos_y + 20, color, font);
    drawString("r: Add a rect Water", start_pos_x, start_pos_y + 40, color, font);
    drawString("i: Show system information", start_pos_x, start_pos_y + 60, color, font);
    drawString("Q/q/ESC: Quit", start_pos_x, start_pos_y + 80, color, font);
    drawString("e: Add dame break", start_pos_x, start_pos_y + 100, color, font);
    glPopMatrix();                      
    glMatrixMode(GL_MODELVIEW);         
    glPopMatrix();                      

}

//----------------------------------------------------------------------
void resizeWindow(int w, int h)
{
    if (h==0)
    {
        h=1;
    }
    aspect = w / (h * 1.0);
    window_width = w;
    window_height = h;
    ps->system->getRenderer()->setWindowDimensions(w,h);
    glutPostRedisplay();
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
