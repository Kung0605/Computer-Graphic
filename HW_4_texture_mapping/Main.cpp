#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "./Library/SOIL/include/SOIL.h"
#include "./Library/glut/glut.h"
          
#pragma comment( lib, "./Library/glut/glut32.lib" )
#pragma comment( lib, "./Library/SOIL/lib/SOIL.lib" )
#define type 0 // 0: buddha, 1: tiger
// Global texture ID
GLuint textures[2];

// Parameter for View angles
float angleX = 0.0f;
float angleY = 0.0f;
int lastX, lastY;
int i = 0;
struct vertex {
    float x;
    float y;
    float z;
};
struct vertex_tex {
    float u;
    float v;
};
struct normal {
	float x;
	float y;
	float z;
};
struct vertex_index {
    int x;
    int y;
    int z;
};
struct vertex_tex_index {
	int u;
	int v;
};
struct face {
    vertex_index v1, v2, v3;
    vertex_tex_index vt1, vt2, vt3;
};
struct material {
    float ambient[3];
    float diffuse[3];
    float specular[3];
};
struct object {
    std::vector<vertex> vertices;
    std::vector<vertex_tex> vertices_tex;
    //std::vector<vertex_normal> vertices_normal;
    std::vector<face> faces;
    material mat;
};
object buddha, tiger;
float size = 0;
material set_material(std::string filename) {
    material mat;
    std::fstream file;
    file.open(filename);
    std::string str;
    while (std::getline(file, str)) {
        if (str[0] == 'K' && str[1] == 'a') {
            sscanf_s(str.c_str(), "Ka %f %f %f", &mat.ambient[0], &mat.ambient[1], &mat.ambient[2]);
            printf("%f %f %f\n", mat.ambient[0], mat.ambient[1], mat.ambient[2]);
        }
        else if (str[0] == 'K' && str[1] == 'd') {
            sscanf_s(str.c_str(), "Kd %f %f %f", &mat.diffuse[0], &mat.diffuse[1], &mat.diffuse[2]);
            printf("%f %f %f\n", mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        }
        else if (str[0] == 'K' && str[1] == 's') {
            sscanf_s(str.c_str(), "Ks %f %f %f", &mat.specular[0], &mat.specular[1], &mat.specular[2]);
            printf("%f %f %f\n", mat.specular[0], mat.specular[1], mat.specular[2]);
        }
        else {
			continue;
		}
    }
    return mat;
}
object load_obj(std::string filename) {
    object result;
    std::ifstream file(filename);
    std::string str;
    while (std::getline(file, str)) {
        if (str[0] == 'm' && str[1] == 't' && str[2] == 'l') {
            int start_pos = str.find(' ') + 1;
            std::string filename = str.substr(start_pos, str.length() - start_pos);
            result.mat = set_material(filename);
            //printf("%f %f %f\n", result.mat.ambient, result.mat.diffuse, result.mat.specular);
        }
        else if (str[0] == 'v' && str[1] == ' ') {
            vertex v;
            sscanf_s(str.c_str(), "v %f %f %f", &v.x, &v.y, &v.z);
            //printf("v %f %f %f\n", v.x, v.y, v.z);
            if (abs(v.x) > size) size = abs(v.x);
            if (abs(v.y) > size) size = abs(v.y);
            if (abs(v.z) > size) size = abs(v.z);
            result.vertices.push_back(v);
        }
        else if (str[0] == 'v' && str[1] == 't') {
            vertex_tex vt;
            sscanf_s(str.c_str(), "vt %f %f", &vt.u, &vt.v);
            //printf("vt %f %f\n", vt.u, vt.v);
            result.vertices_tex.push_back(vt);
        }
        /*else if (str.substr(0, 2) == "vn") {
            vertex_normal vn;
            sscanf_s(str.c_str(), "vn %f %f %f", &vn.x, &vn.y, &vn.z);
            result.vertices_normal.push_back(vn);
        }*/
        else if (str[0] == 'f') {
            face f;
            int a, b, c;
            sscanf_s(str.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &f.v1.x, &f.vt1.u, &a, &f.v2.x, &f.vt2.u, &b, &f.v3.x, &f.vt3.u, &c);
            //printf("f %d/%d/%d %d/%d/%d %d/%d/%d\n", f.v1.x, f.vt1.u, a, f.v2.x, f.vt2.u, b, f.v3.x, f.vt3.u, c);
            result.faces.push_back(f);
        }
        else {
            continue;
        }
    }
    //printf("size: %f\n", size);
    return result;
}
// Initialization of shading and texture
void init() {

    glDisable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //glEnable(GL_CULL_FACE);

    // Enable shading
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat light_ambient0[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat light_diffuse0[] = { 0.85f, 0.1f, 0.1f, 1.0f };
    GLfloat light_specular0[] = { 1.0f, 0.8f, 0.6f, 1.0f };
    GLfloat light_position0[] = { 0.0f, 0.0f, 1.0f, 0.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);

    GLfloat light_ambient1[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat light_diffuse1[] = { 0.5f, 0.2f, 0.1f, 1.0f };
    GLfloat light_specular1[] = { 0.6f, 1.0f, 0.8f, 1.0f };
    GLfloat light_position1[] = { 0.0f, 0.0f, 1.0f, 0.0f };

    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

    GLfloat light_ambient2[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat light_diffuse2[] = { 0.5f, 0.1f, 0.3f, 1.0f };
    GLfloat light_specular2[] = { 0.8f, 0.6f, 1.0f, 1.0f };
    GLfloat light_position2[] = { 1.0f, 0.0f, 0.0f, 0.0f };

    glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse2);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular2);
    glLightfv(GL_LIGHT2, GL_POSITION, light_position2);

    GLfloat light_ambient3[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat light_diffuse3[] = { 0.5f, 0.2f, 0.2f, 1.0f };
    GLfloat light_specular3[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_position3[] = { 0.0f, 1.0f, 0.0f, 0.0f };

    glLightfv(GL_LIGHT3, GL_AMBIENT, light_ambient3);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, light_diffuse3);
    glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular3);
    glLightfv(GL_LIGHT3, GL_POSITION, light_position3);
    // Load textures
    textures[0] = SOIL_load_OGL_texture("./images/buddha-atlas.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    textures[1] = SOIL_load_OGL_texture("./images/tiger-atlas.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    if (textures[0] == 0 || textures[1] == 0) {
        printf("Error loading texture\n");
    }
    
    if (type == 0)
        buddha = load_obj("./buddha.obj");
    else 
        tiger = load_obj("./tiger.obj");
    // Enable 2D texture
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (type == 0) {
        GLfloat mat_ambient[] = { buddha.mat.ambient[0], buddha.mat.ambient[1], buddha.mat.ambient[2], 1.0f};
        GLfloat mat_diffuse[] = { buddha.mat.diffuse[0], buddha.mat.diffuse[1], buddha.mat.diffuse[2], 1.0f};
        GLfloat mat_specular[] = { buddha.mat.specular[0], buddha.mat.specular[1], buddha.mat.specular[2], 1.0f};
        GLfloat mat_shininess[] = { 50.0f };

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        printf("%f %f %f", mat_ambient[0], mat_ambient[1], mat_ambient[2]);
    }
    else if (type == 1) {
        GLfloat mat_ambient[] = { tiger.mat.ambient[0], tiger.mat.ambient[1], tiger.mat.ambient[2], 1.0f };
        GLfloat mat_diffuse[] = { tiger.mat.diffuse[0], tiger.mat.diffuse[1], tiger.mat.diffuse[2], 1.0f };
        GLfloat mat_specular[] = { tiger.mat.specular[0], tiger.mat.specular[1], tiger.mat.specular[2], 1.0f };
        GLfloat mat_shininess[] = { 50.0f };
        printf("%f %f %f", mat_ambient[0], mat_ambient[1], mat_ambient[2]);

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    }
    
}

void reshape(int w, int h) {
    if (h == 0) h = 1; // Divide by zero
    float ratio = (float)w / (float)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, w, h);
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}
normal get_normal(vertex v1, vertex v2, vertex v3) {
    float x1 = v2.x - v1.x, y1 = v2.y - v1.y ,z1 = v2.z - v1.z;
    float x2 = v3.x - v1.x, y2 = v3.y - v1.y, z2 = v3.z - v1.z;
    normal n;
    n.x = y1 * z2 - z1 * y2;
    n.y = z1 * x2 - x1 * z2;
    n.z = x1 * y2 - y1 * x2;
    return n;
}
void drawobject(object obj, GLenum mode, std::string object) {
    //printf("%f %f %f\n", obj.mat.ambient[1], obj.mat.diffuse[1], obj.mat.specular[1]);
    for (face& face : obj.faces) {
        glBindTexture(GL_TEXTURE_2D, (object == "buddha" ? textures[0] : textures[1]));
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
        glMaterialfv(GL_FRONT, GL_AMBIENT, obj.mat.ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, obj.mat.diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, obj.mat.specular);
        normal n = get_normal(obj.vertices[face.v1.x - 1], obj.vertices[face.v2.x - 1], obj.vertices[face.v3.x - 1]);
        glBegin(GL_TRIANGLES);
        //printf("%f, %f, %f\n", n.x, n.y, n.z);
        glNormal3f(n.x, n.y, n.z);
        glTexCoord2f(obj.vertices_tex[face.vt1.u - 1].u, obj.vertices_tex[face.vt1.u - 1].v); glVertex3f(obj.vertices[face.v1.x - 1].x / size, obj.vertices[face.v1.x - 1].y / size, obj.vertices[face.v1.x - 1].z / size);
        glTexCoord2f(obj.vertices_tex[face.vt2.u - 1].u, obj.vertices_tex[face.vt2.u - 1].v); glVertex3f(obj.vertices[face.v2.x - 1].x / size, obj.vertices[face.v2.x - 1].y / size, obj.vertices[face.v2.x - 1].z / size);
        glTexCoord2f(obj.vertices_tex[face.vt3.u - 1].u, obj.vertices_tex[face.vt3.u - 1].v); glVertex3f(obj.vertices[face.v3.x - 1].x / size, obj.vertices[face.v3.x - 1].y / size, obj.vertices[face.v3.x - 1].z / size);
        glEnd();
    }
}
//void drawBox(GLenum mode) {
//    // Set texture mode
//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
//
//    // front
//    glBindTexture(GL_TEXTURE_2D, textures[1]);
//    glBegin(GL_QUADS);
//    glNormal3f(0.0, 0.0, 1.0);
//    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, 1.0);
//    glTexCoord2f(1.0, 0.0); glVertex3f(1.0, -1.0, 1.0);
//    glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, 1.0);
//    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, 1.0);
//    glEnd();
//
//    // back
//    glBindTexture(GL_TEXTURE_2D, textures[1]);
//    glBegin(GL_QUADS);
//    glNormal3f(0.0, 0.0, -1.0);
//    glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0, -1.0);
//    glTexCoord2f(1.0, 1.0); glVertex3f(-1.0, 1.0, -1.0);
//    glTexCoord2f(0.0, 1.0); glVertex3f(1.0, 1.0, -1.0);
//    glTexCoord2f(0.0, 0.0); glVertex3f(1.0, -1.0, -1.0);
//    glEnd();
//
//    // left
//    glBindTexture(GL_TEXTURE_2D, textures[1]);
//    glBegin(GL_QUADS);
//    glNormal3f(-1.0, 0.0, 0.0);
//    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, -1.0);
//    glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0, 1.0);
//    glTexCoord2f(1.0, 1.0); glVertex3f(-1.0, 1.0, 1.0);
//    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, -1.0);
//    glEnd();
//
//    // right
//    glBindTexture(GL_TEXTURE_2D, textures[1]);
//    glBegin(GL_QUADS);
//    glNormal3f(1.0, 0.0, 0.0);
//    glTexCoord2f(1.0, 0.0); glVertex3f(1.0, -1.0, -1.0);
//    glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, -1.0);
//    glTexCoord2f(0.0, 1.0); glVertex3f(1.0, 1.0, 1.0);
//    glTexCoord2f(0.0, 0.0); glVertex3f(1.0, -1.0, 1.0);
//    glEnd();
//
//    // top
//    glBindTexture(GL_TEXTURE_2D, textures[0]);
//    glBegin(GL_QUADS);
//    glNormal3f(0.0, 1.0, 0.0);
//    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, -1.0);
//    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, 1.0, 1.0);
//    glTexCoord2f(1.0, 0.0); glVertex3f(1.0, 1.0, 1.0);
//    glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, -1.0);
//    glEnd();
//
//    // bottom
//    glBindTexture(GL_TEXTURE_2D, textures[0]);
//    glBegin(GL_QUADS);
//    glNormal3f(0.0, -1.0, 0.0);
//    glTexCoord2f(1.0, 1.0); glVertex3f(-1.0, -1.0, -1.0);
//    glTexCoord2f(0.0, 1.0); glVertex3f(1.0, -1.0, -1.0);
//    glTexCoord2f(0.0, 0.0); glVertex3f(1.0, -1.0, 1.0);
//    glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0, 1.0);
//    glEnd();
//}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // View setting
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    // Draw content
    if (type == 0)
        drawobject(buddha, GL_BLEND, "buddha"); // Test GL_MODULATE, GL_REPLACE and GL_BLEND to check different results
    else if (type == 1)
        drawobject(tiger, GL_BLEND, "tiger"); // Test GL_MODULATE, GL_REPLACE and GL_BLEND to check different results
    else
        printf("error");
    glutSwapBuffers();
}

// Change viewpoint
void mouseMove(int x, int y) {
    angleX += (y - lastY);
    angleY += (x - lastX);
    lastX = x;
    lastY = y;
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        lastX = x;
        lastY = y;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Texture Example");

    init();

    glutDisplayFunc(display);
    glutMotionFunc(mouseMove);
    glutMouseFunc(mouseButton);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
