
#include <QOpenGLFunctions_4_3_Core>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <RTPSettings.h>

#include "Render.h"
#include "util.h"
#include "stb_image.h" 
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" 

using namespace std;


namespace rtps
{

    Render::Render(QOpenGLBuffer pos_vbo, QOpenGLBuffer col_vbo, CL* cli, RTPSettings* _settings) :
        num(0),
        m_pos_vbo(pos_vbo),
        m_col_vbo(col_vbo),
        m_box_vbo(QOpenGLBuffer::VertexBuffer),
        m_box_index(QOpenGLBuffer::IndexBuffer)
    {
        this->settings = _settings;
     
        shader_source_dir = settings->GetSettingAs<string>("rtps_path");
        shader_source_dir += "/shaders";

        initShaderProgram();
        initBoxBuffer();
        initParticleBuffer();

        rtype = POINTS;
        this->cli=cli;

        near_depth=0.;
        far_depth=1.;
        write_framebuffers = false;
        GLubyte col1[] = {0,0,0,255};
        GLubyte col2[] = {255,255,255,255};

        blending = settings->GetSettingAs<bool>("render_use_alpha");
        setupTimers();
    }

    //----------------------------------------------------------------------
    Render::~Render()
    {
        printf("Render destructor\n");
        for (map<ShaderType,GLuint>::iterator i = glsl_program.begin();i!=glsl_program.end();i++)
        {
            glDeleteProgram(i->second);
        }


        for (map<string,GLuint>::iterator i = gl_framebuffer_texs.begin();i!=gl_framebuffer_texs.end();i++)
        {
            glDeleteTextures(1,&(i->second));
        }
        if (rbos.size())
        {
            glDeleteRenderbuffers(rbos.size() ,&rbos[0]);
        }
        if (fbos.size())
        {
            glDeleteFramebuffers(fbos.size(),&fbos[0]);
        }
    }

    void Render::initBoxBuffer()
    {
        float4 min = settings->grid->getBndMin();
        float4 max = settings->grid->getBndMax();

        static float position_buf[] = { 
            min.x, min.y, min.z,
            min.x, min.y, max.z,
            max.x, min.y, max.z,
            max.x, min.y, min.z,

            min.x, max.y, min.z,
            min.x, max.y, max.z,
            max.x, max.y, max.z,
            max.x, max.y, min.z,
        };

        static unsigned int index_buf[] = {
            0, 1,
            1, 2,
            2, 3,
            3, 0,

            0, 4,
            1, 5,
            2, 6,
            3, 7,

            4, 5,
            5, 6,
            6, 7,
            7, 4
        };

        m_box_vao.create();
        m_box_vao.bind();

        m_box_vbo.create();
        m_box_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_box_vbo.bind();
        m_box_vbo.allocate(&position_buf[0], sizeof(position_buf));
        m_basic_program.enableAttributeArray(0);
        m_basic_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);
        
        m_box_index.create();
        m_box_index.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_box_index.bind();
        m_box_index.allocate(&index_buf[0], sizeof(index_buf));

        m_box_vao.release();
    }

    void Render::initParticleBuffer()
    {
        m_particle_vao.create();
        m_particle_vao.bind();
        m_pos_vbo.bind();
        m_particle_program.enableAttributeArray(0);
        m_particle_program.setAttributeBuffer(0, GL_FLOAT, 0, 4);
        m_particle_vao.release();
    }

    void Render::initShaderProgram()
    {
       assert( m_basic_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/basic.vert").c_str()));
        assert( m_basic_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/basic.frag").c_str()));
        assert(m_basic_program.link());

assert( m_particle_program.addShaderFromSourceFile(QOpenGLShader::Vertex, (shader_source_dir + "/basic.vert").c_str()));
        assert( m_particle_program.addShaderFromSourceFile(QOpenGLShader::Fragment, (shader_source_dir + "/basic.frag").c_str()));
        assert(m_particle_program.link());

    }

    void Render::drawArrays()
    {
        glBindBuffer(GL_ARRAY_BUFFER, col_vbo);
        glColorPointer(4, GL_FLOAT, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
        glVertexPointer(4, GL_FLOAT, 0, 0);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        //Need to disable these for blender
        glDisableClientState(GL_NORMAL_ARRAY);
        glDrawArrays(GL_POINTS, 0, num);

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    void Render::renderBox()
    {
        QMatrix4x4 matrix;
        matrix.perspective(60.f, window_width/(window_height * 1.0f), 0.1f, 100.0f);
        matrix.translate(0, 0, -10);

        m_basic_program.bind();
        GLuint uniform_matrix = m_basic_program.uniformLocation("matrix");
        m_basic_program.setUniformValue(uniform_matrix, matrix);
        m_box_vao.bind();
        m_opengl_funcs->glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        m_basic_program.release();
    }

    void Render::render()
    {
        m_opengl_funcs->glEnable(GL_POINT_SMOOTH);
        m_opengl_funcs->glPointSize(5.0f);

        QMatrix4x4 matrix;
        matrix.perspective(60.f, window_width/(window_height * 1.0f), 0.1f, 100.0f);
        matrix.translate(0, 0, -10);

        m_particle_program.bind();
        m_particle_vao.bind();

        GLuint uniform_matrix = m_basic_program.uniformLocation("matrix");
        m_particle_program.setUniformValue(uniform_matrix, matrix);
        m_opengl_funcs->glDrawArrays(GL_POINTS, 0, num);
        m_opengl_funcs->glFinish(); 

        m_particle_vao.release();
        m_particle_program.release();
    }

    //void Render::render()
    //{
    //    timers["render"]->start();

    //    glPushAttrib(GL_ALL_ATTRIB_BITS);
    //    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    //    glDepthMask(GL_TRUE);
	//	glEnable(GL_LIGHTING);
	//	glDisable(GL_LIGHTING);
    //    glDepthMask(GL_FALSE);

    //    if (blending)
    //    {
    //        glDepthMask(GL_FALSE);
    //        glEnable(GL_BLEND);
    //        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //    }

    //    glEnable(GL_POINT_SMOOTH); 
    //    glPointSize(15.0f);

    //    drawArrays();

    //    glDepthMask(GL_TRUE);

    //    glDisable(GL_LIGHTING);

    //    glPopClientAttrib();
    //    glPopAttrib();
    //    if (blending)
    //    {
    //        glDisable(GL_BLEND);
    //    }
    //    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //    timers["render"]->end();
    //    glFinish();
    //}

    void Render::writeBuffersToDisk()
    {
        write_framebuffers = true;
    }


    void Render::writeFramebufferTextures() 
    {
        for (map<string,GLuint>::iterator i = gl_framebuffer_texs.begin();i!=gl_framebuffer_texs.end();i++)
        {
            string s(i->first);
            s+=".png";
            writeTexture(i->second, s.c_str());
        }
    }

    void Render::convertDepthToRGB(const GLfloat* depth, GLuint size, GLubyte* rgba) const
    {
        GLfloat minimum = 1.0f;
        for (GLuint i = 0;i<size;i++)
        {
            if (minimum>depth[i])
            {
                minimum = depth[i];
            }
        }
        GLfloat one_minus_min = 1.f-minimum;
        for (GLuint i = 0;i<size;i++)
        {
            for (GLuint j = 0;j<3;j++)
            {
                rgba[(i*4)+j]=(GLubyte)(((depth[i]-minimum)/one_minus_min) *255U);
            }
            rgba[(i*4)+3] = 255U; //no transparency;
        }
    }

    int Render::writeTexture(GLuint tex, const char* filename) 
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        GLubyte* image = new GLubyte[window_width*window_height*4];
        if (!strcmp(filename,"depth.png") || !strcmp(filename,"depth2.png"))
        {
            GLfloat* fimg = new GLfloat[window_width*window_height];
            glGetTexImage(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,GL_FLOAT,fimg);
            convertDepthToRGB(fimg,window_width*window_height,image);
	    delete[] fimg;
        }
        else
        {
            glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
        }
        if (!stbi_write_png(filename,window_width,window_height,4,(void*)image,0))
        {
            printf("failed to write image %s",filename);
            return -1;
        }

        glBindTexture(GL_TEXTURE_2D,0);
        delete[] image;

        return 0;
    }


    void Render::orthoProjection()
    {
        glMatrixMode(GL_PROJECTION);                    // Select Projection
        glPushMatrix();                         // Push The Matrix
        glLoadIdentity();                       // Reset The Matrix
        gluOrtho2D( 0,1,0,1);
        glMatrixMode(GL_MODELVIEW);                 // Select Modelview Matrix
        glPushMatrix();                         // Push The Matrix
        glLoadIdentity();                       // Reset The Matrix
    }

    void Render::perspectiveProjection()
    {
        glMatrixMode( GL_PROJECTION );                  // Select Projection
        glPopMatrix();                          // Pop The Matrix
        glMatrixMode( GL_MODELVIEW );                   // Select Modelview
        glPopMatrix();                          // Pop The Matrix
    }

    void Render::fullscreenQuad()
    {
        orthoProjection();
        glBegin(GL_QUADS);
        glTexCoord2f(0.f,0.f);
        glVertex2f(0.f,0.f);

        glTexCoord2f(1.f,0.f);
        glVertex2f(1.f,0.f);

        glTexCoord2f(1.f,1.f);
        glVertex2f(1.f,1.f);

        glTexCoord2f(0.f,1.f);
        glVertex2f(0.f,1.f);
        glEnd();
        perspectiveProjection();
    }

	//----------------------------------------------------------------------
    void Render::renderPointsAsSpheres()
    {

        printf("run this\n");
        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

        glUseProgram(glsl_program[SPHERE_SHADER]);
        glUniform1f( glGetUniformLocation(glsl_program[SPHERE_SHADER], "pointScale"), ((float)window_width) / tanf(65. * (0.5f * 3.1415926535f/180.0f)));

        float radius_scale = settings->getRadiusScale(); 
        glUniform1f( glGetUniformLocation(glsl_program[SPHERE_SHADER], "pointRadius"), particle_radius*radius_scale ); 
        glUniform1f( glGetUniformLocation(glsl_program[SPHERE_SHADER], "near"), near_depth );
        glUniform1f( glGetUniformLocation(glsl_program[SPHERE_SHADER], "far"), far_depth );


        drawArrays();

        glUseProgram(0);

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glDisable(GL_POINT_SPRITE);
    }
	//----------------------------------------------------------------------


        void Render::render_table(float4 min, float4 max)
    {

        glEnable(GL_DEPTH_TEST);
        glColor4f(0.0f, 0.4f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        float4 scale = float4((0.25f)*(max.x-min.x),(0.25f)*(max.y-min.y),(0.25f)*(max.z-min.z),0.0f);
        glTexCoord2f(0.f,0.f);
        glVertex3f(-10000., -10000., min.z);
        glTexCoord2f(1.f,0.f);
        glVertex3f(10000., -10000., min.z); 
        glTexCoord2f(1.f,1.f);
        glVertex3f(10000., 10000., min.z);
        glTexCoord2f(0.f,1.f);
        glVertex3f(-10000., 10000., min.z);
        glEnd();
        m_opengl_funcs->glBindTexture(GL_TEXTURE_2D,0);
    }

	//----------------------------------------------------------------------
    int Render::generateCheckerBoardTex(GLubyte* color1,GLubyte* color2,int num_squares, int length)
    {
        unsigned int imageSize = length*length;
        GLubyte* image = new GLubyte[imageSize*4];
        memset(image,0,imageSize);
        int sq_size = length/num_squares;
        GLubyte* col;
        for (unsigned int i = 0; i<imageSize; i++)
        {
            if ((i/sq_size)%2 && (i/sq_size))
            {
                col = color1;
            }
            else
            {
                col = color2;
            }
            for(int j = 0; j<4; j++)
            {
                image[(i*4)+j] = col[j];
            }
        }

        glGenTextures(1, &gl_textures["checker_board"]);
        glBindTexture(GL_TEXTURE_2D, gl_textures["checker_board"]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, length, length, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image);
		delete[] image;
        return 0; //success
    }

    //----------------------------------------------------------------------
    GLuint Render::compileShaders(const char* vertex_file, const char* fragment_file,
            const char* geometry_file, GLenum* geom_param, GLint* geom_value, int geom_param_len)
    {


        char *vertex_shader_source = NULL,*fragment_shader_source= NULL,*geometry_shader_source=NULL;
        int vert_size,frag_size,geom_size;
        if (vertex_file)
        {
            vertex_shader_source = file_contents(vertex_file,&vert_size);
            if (!vertex_shader_source)
            {
                printf("Vertex shader file not found or is empty! Cannot compile shader");
                return -1;
            }
        }
        else
        {
            printf("No vertex file specified! Cannot compile shader!");
            return -1;
        }

        if (fragment_file)
        {
            fragment_shader_source = file_contents(fragment_file,&frag_size);
            if (!fragment_shader_source)
            {
                printf("Fragment shader file not found or is empty! Cannot compile shader");
                free(vertex_shader_source);
                return -1;
            }
        }
        else
        {
            printf("No fragment file specified! Cannot compile shader!");
            free(vertex_shader_source);
            return -1;
        }

        if (geometry_file)
        {
            geometry_shader_source = file_contents(fragment_file,&frag_size);
            if (!geometry_shader_source)
            {
                printf("Geometry shader file not found or is empty! Cannot compile shader");
                free(vertex_shader_source);
                free(fragment_shader_source);
                return -1;
            }
        }

        GLint len;
        GLuint program = glCreateProgram();

        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, (const GLchar**)&vertex_shader_source, 0);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            char log[1024];
            glGetShaderInfoLog(vertex_shader, 1024, 0, log);
            printf("Vertex Shader log:\n %s\n", log);
        }
        glAttachShader(program, vertex_shader);

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, (const GLchar**)&fragment_shader_source, 0);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            char log[1024];
            glGetShaderInfoLog(fragment_shader, 1024, 0, log);
            printf("Fragment Shader log:\n %s\n", log);
        }
        glAttachShader(program, fragment_shader);


        GLuint geometry_shader=0;
        if (geometry_shader_source)
        {
            geometry_shader = glCreateShader(GL_GEOMETRY_SHADER_EXT);
            glShaderSource(geometry_shader, 1, (const GLchar**)&geometry_shader_source, 0);
            glCompileShader(geometry_shader);
            glGetShaderiv(geometry_shader, GL_INFO_LOG_LENGTH, &len);
            printf("geometry len %d\n", len);
            if (len > 0)
            {
                char log[1024];
                glGetShaderInfoLog(geometry_shader, 1024, 0, log);
                printf("Geometry Shader log:\n %s\n", log);
            }
            glAttachShader(program, geometry_shader);
            for (int i = 0;i < geom_param_len; i++)
            {
               //glProgramParameteri(program,geom_param[i],geom_value[i]);
            }
        }

        glLinkProgram(program);

        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success)
        {
            char temp[256];
            glGetProgramInfoLog(program, 256, 0, temp);
            printf("Failed to link program:\n%s\n", temp);
            glDeleteProgram(program);
            program = 0;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        if (geometry_shader)
        {
            glDeleteShader(geometry_shader);
        }
        free(vertex_shader_source);
        free(fragment_shader_source);
        free(geometry_shader_source);

        return program;
    }

    int Render::setupTimers()
    {
        int time_offset = 5;
        timers["render"] = new EB::Timer("Render call", time_offset);
		return 0;
    }

    void Render::printTimers()
    {
        timers.printAll();
    }


    int Render::generateCircleTexture(GLubyte r, GLubyte g, GLubyte b, GLubyte alpha, int diameter)
    {
        unsigned int imageSize = diameter*diameter*4;
        unsigned int radius = diameter/2;
        GLubyte* image = new GLubyte[imageSize];
        memset(image,0,imageSize);

        for (unsigned int i = 0; i<imageSize; i+=4)
        {
            int x = ((i/4)%diameter)-(radius);
            int y = (radius)-((i/4)/diameter);
            if ((x*x)+(y*y)<=(radius*radius))
            {
                image[i] = r;
                image[i+1] = g;
                image[i+2] = b;
                image[i+3] = alpha;
            }
            else
            {
                image[i] =0;
                image[i+1] =0;
                image[i+2] =0;
                image[i+3] =0;
            }
        }

        glGenTextures(1, &gl_textures["circle"]);
        glBindTexture(GL_TEXTURE_2D, gl_textures["circle"]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diameter, diameter, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image);
        delete[] image;
        return 0; //success
    }

    int Render::loadTexture(string texture_file, string texture_name)
    {

        printf("LOAD TEXTURE!!!!!!!!!!!!!!\n");
        int w,h,channels;
        int force_channels = 0;

        unsigned char *im = stbi_load( texture_file.c_str(), &w, &h, &channels, force_channels );
        printf("after load w: %d h: %d channels: %d\n", w, h, channels);
        printf("im looking for the image at %s\n", texture_file.c_str());
        if (im == NULL)
        {
            printf("fail!: %s\n", stbi_failure_reason());
            printf("WTF\n");
        }

        glGenTextures(1, &gl_textures[texture_name]);
        glBindTexture(GL_TEXTURE_2D, gl_textures[texture_name]);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        if(channels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, &im[0]);
        }
        else if (channels == 4)
        {
             glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, &im[0]);
        }

        glBindTexture(GL_TEXTURE_2D,0);
        free(im);
        return 0; 
    }

    void Render::deleteFramebufferTextures()
    {

    }

    void Render::createFramebufferTextures()
    {

    }

    void Render::setWindowDimensions(GLuint width, GLuint height)
    {
        window_width = width;
        window_height = height;
    }

    void Render::setParticleRadius(float pradius)
    {
        particle_radius = pradius;
    }

}


