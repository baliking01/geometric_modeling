#include "scene.h"

#include <obj/load.h>
#include <obj/draw.h>
#include <stdlib.h>
#include <math.h>

void init_scene(Scene* scene)
{
    scene->dim_n = 10;
    scene->dim_m = 10;
    scene->res = 2;
    scene->points = (Vertex*)malloc(scene->dim_n * scene->dim_m * sizeof(Vertex));
    scene->dz = (double*)malloc(scene->dim_n * scene->dim_m * sizeof(double));
    scene->disp_points = (Vertex*)malloc(scene->dim_n * scene->res * scene->dim_m * scene->res * sizeof(Vertex));

    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;
    for (int i = 0; i < dim_n; i++) {
        for (int j = 0; j < dim_m; j++) {
            scene->points[i*dim_m + j] = (Vertex){i, j, rand() % 4};
            scene->dz[i*dim_m + j] = scene->points[i*dim_m + j].z;
        }
    }
}

void set_lighting()
{
    float ambient_light[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    float diffuse_light[] = { 1.0f, 1.0f, 1.0, 1.0f };
    float specular_light[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float position[] = { 0.0f, 0.0f, 10.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular_light);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
}

void set_material(const Material* material)
{
    float ambient_material_color[] = {
        material->ambient.red,
        material->ambient.green,
        material->ambient.blue
    };

    float diffuse_material_color[] = {
        material->diffuse.red,
        material->diffuse.green,
        material->diffuse.blue
    };

    float specular_material_color[] = {
        material->specular.red,
        material->specular.green,
        material->specular.blue
    };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_material_color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_material_color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_material_color);

    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &(material->shininess));
}

// TODO: implement lookup table
unsigned long binom(int n, int k)
{
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    unsigned long res = 1;
    for (int i = 1; i <= k; i++) {
        res *= n - (k - i);
        res /= i;
    }
    return res;
}

// compute the k-th bernstein polynomial of n-th degree at t
double bernstein(int n, int k, double t)
{
    return binom(n, k) * pow(t, k) * pow(1 - t, n - k);
}

Vertex bezier_surface(Scene *scene, double u, double v)
{
    Vertex sum = {0};
    double B;
    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;

    for (int i = 0; i < dim_n; i++) {
        for (int j = 0; j < dim_m; j++) {
            B = bernstein(dim_n - 1, i, u) * bernstein(dim_m - 1, j, v);
            sum.x += scene->points[i*dim_m + j].x * B;
            sum.y += scene->points[i*dim_m + j].y * B;
            sum.z += scene->points[i*dim_m + j].z * B;
        }
    } 

    return sum;
}

void update_scene(Scene* scene)
{
    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;
    int res = scene->res;

    // oscillate control points
    for (int i = 0; i < dim_n * dim_m; i++) {
        scene->dz[i] += 0.01 + 1/(((rand() % 5) + 1)*10);
        scene->points[i].z = ((sin(scene->dz[i]) + 1) / 2) * 4;
    }

    // compute bezier surface using control points
    double u, v;
    for (int i = 0; i < dim_n * res; i++) {
        for (int j = 0; j < dim_m * res; j++) {
            u = (double)i / ((dim_n * res) - 1);
            v = (double)j / ((dim_m * res) - 1);
            scene->disp_points[i*dim_m*res + j] = bezier_surface(scene, u, v);
        }
    }
}

void render_scene(const Scene* scene)
{
    draw_origin();

    // draw bezier surface
    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;
    int res = scene->res;

    Vertex v1, v2, v3;
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < dim_n * res - 1; i++) {
        for (int j = 0; j < dim_m * res - 1; j++) {
            v1 = scene->disp_points[i*dim_m*res + j];
            glColor3f(1.0, 0, 0);
            glVertex3f(v1.x, v1.y, v1.z);

            v2 = scene->disp_points[(i + 1)*dim_m*res + j];
            glColor3f(0, 1.0, 0);
            glVertex3f(v2.x, v2.y, v2.z);

            v3 = scene->disp_points[(i + 1)*dim_m*res + j + 1];
            glColor3f(1.0, 1.0, 0);
            glVertex3f(v3.x, v3.y, v3.z);

            glColor3f(1.0, 0, 1.0);
            glVertex3f(v1.x, v1.y, v1.z);
            glColor3f(0.0, 1.0, 1.0);
            glVertex3f(v3.x, v3.y, v3.z);

            v2 = scene->disp_points[i*dim_m*res + j + 1];
            glColor3f(0, 0, 1.0);
            glVertex3f(v2.x, v2.y, v2.z);

        }
    }

    glEnd();
}

void draw_origin()
{
    glBegin(GL_LINES);

    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);

    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);

    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);

    glEnd();
}

