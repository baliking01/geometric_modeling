#include "scene.h"

#include <obj/load.h>
#include <obj/draw.h>
#include <stdlib.h>
#include <math.h>

#include <stdio.h>

extern unsigned long long int fact[21];

#define MAX_DIM 15
#define MAX_RES 20

void init_scene(Scene* scene)
{   
    scene->texture_id = load_texture("assets/textures/cube.png");
    glBindTexture(GL_TEXTURE_2D, scene->texture_id);


    // material properties
    scene->material.ambient.red = 5.0;
    scene->material.ambient.green = 5.0;
    scene->material.ambient.blue = 5.0;

    scene->material.diffuse.red = 1.0;
    scene->material.diffuse.green = 1.0;
    scene->material.diffuse.blue = 1.0;

    scene->material.specular.red = 3.0;
    scene->material.specular.green = 3.0;
    scene->material.specular.blue = 3.0;

    scene->material.shininess = 0.7;

    init_surface(scene, 5, 4, 10);

    // set visibility
    scene->normals = 0;
    scene->control_polygon = 0;
}

void set_lighting()
{
    float ambient_light[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float diffuse_light[] = { 1.0f, 1.0f, 1.0, 1.0f };
    float specular_light[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float position[] = { 5.0f, 5.0f, 10.0f, 1.0f };

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

unsigned long long int slow_binom(int n, int k)
{
    unsigned long long res = 1;
    for (int i = 1; i <= k; i++) {
        res *= n - (k - i);
        res /= i;
    }
    return res;
}

unsigned long long binom(int n, int k)
{
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (n > 21 || k > 21) return slow_binom(n, k);
    return fact[n] / (fact[k] * fact[n - k]);
}

// compute the k-th bernstein polynomial of n-th degree at t
double bernstein(int n, int k, double t)
{
    return binom(n, k) * pow(t, k) * pow(1 - t, n - k);
}

vec3 bezier_surface(Scene *scene, double u, double v)
{
    vec3 sum = {0};
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

vec3 cross(vec3 a, vec3 b)
{
    vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    // normalize
    double len = sqrt(result.x*result.x + result.y*result.y + result.z*result.z);
    if (len != 0) {
        result.x /= len;
        result.y /= len;
        result.z /= len;
    }

    return result;
}

void update_scene(Scene* scene)
{
    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;
    int res = scene->res;

    // oscillate control points
    for (int i = 0; i < dim_n * dim_m; i++) {
        scene->dz[i] += 0.01 + 1/(((rand() % 5) + 1)*10);
        scene->points[i].z = ((sin(scene->dz[i]) + 1) / 2) * 2;
    }

    // compute bezier surface
    double u, v;
    for (int i = 0; i < dim_n * res; i++) {
        for (int j = 0; j < dim_m * res; j++) {
            u = (double)i / ((dim_n * res) - 1);
            v = (double)j / ((dim_m * res) - 1);
            scene->disp_points[i*dim_m*res + j].pos = bezier_surface(scene, u, v);
        }
    }

    // compute surface normals at f(u, v)
    vec3 left, right, top, bottom;
    vec3 vector_u, vector_v, normal;
    int r, l, t, b;
    for (int i = 0; i < dim_n * res; i++) {
        for (int j = 0; j < dim_m * res; j++) {
            r = i*dim_m*res + j + 1;
            l = i*dim_m*res + j - 1;
            t = (i - 1)*dim_m*res + j;
            b = (i + 1)*dim_m*res + j;

            // ensure correct indexing at edges and corners
            if (i == 0) {
                t = i*dim_m*res + j;
            }
            else if (i == dim_n*res - 1) {
                b = i*dim_m*res + j;
            }

            if (j == 0) {
                l = i*dim_m*res + j;
            }
            else if (j == dim_m*res - 1) {
                r = i*dim_m*res + j;
            }

            top = scene->disp_points[t].pos;
            bottom = scene->disp_points[b].pos;
            right = scene->disp_points[r].pos;
            left = scene->disp_points[l].pos;
    
            vector_u.x = left.x - right.x;
            vector_u.y = left.y - right.y;
            vector_u.z = left.z - right.z;

            vector_v.x = bottom.x - top.x;
            vector_v.y = bottom.y - top.y;
            vector_v.z = bottom.z - top.z;

            normal = cross(vector_u, vector_v);

            scene->disp_points[i*dim_m*res + j].normal = normal;
        }
    }
}

void render_scene(const Scene* scene)
{
    set_material(&scene->material);
    set_lighting();
    draw_origin();

    // draw bezier surface
    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;
    int res = scene->res;

    vertex_t v1, v2, v3, v4;
    vec3 p1, p2;
    glBegin(GL_QUADS);
    for (int i = 0; i < dim_n*res - 1; i++) {
        for (int j = 0; j < dim_m*res - 1; j++) {
            v1 = scene->disp_points[i*dim_m*res + j];
            v2 = scene->disp_points[(i+1)*dim_m*res + j];
            v3 = scene->disp_points[(i+1)*dim_m*res + j+1];
            v4 = scene->disp_points[i*dim_m*res + j+1];

            glColor3f(1, 0, 0);
            glVertex3fv((float*)(&v1.pos));
            glColor3f(0, 1, 0);
            glVertex3fv((float*)(&v2.pos));
            glColor3f(1, 0, 1);
            glVertex3fv((float*)(&v3.pos));
            glColor3f(1, 1, 0);
            glVertex3fv((float*)(&v4.pos));

            glNormal3fv((float*)(&v1.normal));
            glNormal3fv((float*)(&v2.normal));
            glNormal3fv((float*)(&v3.normal));
            glNormal3fv((float*)(&v4.normal));

            glTexCoord2fv((float*)(&v1.texel));
            glTexCoord2fv((float*)(&v2.texel));
            glTexCoord2fv((float*)(&v3.texel));
            glTexCoord2fv((float*)(&v4.texel));
        }
    }
    glEnd();

    // visualize normals

    if (scene->normals) {
        glBegin(GL_LINES);
        for(int i = 0; i < dim_n*res * dim_m*res; i++) {
            v1 = scene->disp_points[i];

            glColor3f(1.0, 1.0, 1.0);
            glVertex3f(v1.pos.x, v1.pos.y, v1.pos.z);
            v1.pos.x += v1.normal.x;
            v1.pos.y += v1.normal.y;
            v1.pos.z += v1.normal.z;
            glVertex3f(v1.pos.x, v1.pos.y, v1.pos.z);
        }
        glEnd();
    }
    

    // visualize control polygon

    if (scene->control_polygon) {
        glBegin(GL_LINES);
        for(int i = 0; i < dim_n; i++) {
            for (int j = 0; j < dim_m - 1; j++) {
                p1 = scene->points[i*dim_m + j];
                p2 = scene->points[i*dim_m + j+1];

                glColor3f(1.0, 1.0, 1.0);
                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p2.x, p2.y, p2.z);
            }
        }
        glEnd();

        glBegin(GL_LINES);
        for(int i = 0; i < dim_n - 1; i++) {
            for (int j = 0; j < dim_m; j++) {
                p1 = scene->points[i*dim_m + j];
                p2 = scene->points[(i+1)*dim_m + j];

                glColor3f(1.0, 1.0, 1.0);
                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p2.x, p2.y, p2.z);
            }
        }
        glEnd();
    }
    
}

void init_surface(Scene *scene, int n, int m, int r)
{
    // surface parameters
    scene->dim_n = n;
    scene->dim_m = m;
    scene->res = r;

    // populate point control and display points
    size_t n_elements = scene->dim_n * scene->dim_m;
    scene->points = (vec3*)malloc(n_elements * sizeof(vec3));
    scene->dz = (double*)malloc(n_elements * sizeof(double));

    n_elements = (scene->dim_n * scene->res) * (scene->dim_m * scene->res);
    scene->disp_points = (vertex_t*)malloc(n_elements * sizeof(vertex_t));

    generate_surface(scene);
    premap_texture(scene);
    //precompute_bezier(scene);
}


void generate_surface(Scene *scene)
{
    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;
    for (int i = 0; i < dim_n; i++) {
        for (int j = 0; j < dim_m; j++) {
            scene->points[i*dim_m + j] = (vec3){i, j, 10/((rand() % 10) + 1)};
            //scene->points[i*dim_m + j] = (vec3){i, j, 0};
            scene->dz[i*dim_m + j] = scene->points[i*dim_m + j].z;
        }
    }
}

void premap_texture(Scene *scene)
{
    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;
    int res = scene->res;
    double u, v;
    for (int i = 0; i < dim_n * res; i++) {
        for (int j = 0; j < dim_m * res; j++) {
            u = (double)i / ((dim_n * res) - 1);
            v = (double)j / ((dim_m * res) - 1);
            scene->disp_points[i*dim_m*res + j].texel = (texel_t){u, v};
        }
    }
}

// change the size of dimension dim by size
// eg.: 3 -> change by -1 -> 2
void change_dim(Scene *scene, int target_dim, int size)
{
    int dim_n = scene->dim_n;
    int dim_m = scene->dim_m;
    int res = scene->res;

    // first dimension, n
    if (target_dim == 1) {
        dim_n += size;
        if (dim_n > MAX_DIM || dim_n < 2) {
            return;
        }
    }
    // second dimension, m
    else if (target_dim == 2) {
        dim_m += size;
        if (dim_m > MAX_DIM || dim_m < 2) {
            return;
        }
    }
    else {
        return;
    }
    
    // new array size
    int n_elements = dim_n * dim_m;
    vec3 *new_points = (vec3*)realloc(scene->points, n_elements * sizeof(vec3));
    double *new_dz = (double*)realloc(scene->dz, n_elements * sizeof(double));

    n_elements = (dim_n * res) * (dim_m * res);
    vertex_t *new_disp = (vertex_t*)realloc(scene->disp_points, n_elements * sizeof(vertex_t));

    if (new_points == NULL ||
        new_dz     == NULL ||
        new_disp   == NULL) {
        // if the allocation fails, leave the size as is
        // revert previous dimensional changes
        printf("Reallocation failed!\n");
        return;
    }

    // if the allocation succeeds, save the new blocks
    scene->dim_n = dim_n;
    scene->dim_m = dim_m;
    scene->res = res;

    scene->points = new_points;
    scene->dz = new_dz;
    scene->disp_points = new_disp;
    
    // regenerate the surface
    generate_surface(scene);
    premap_texture(scene);
    printf("N:%d, M:%d\n", scene->dim_n, scene->dim_m);
}

void toggle_control_polygon(Scene *scene)
{
    scene->control_polygon = ~(scene->control_polygon);
}

void toggle_normals(Scene *scene)
{
    scene->normals = ~(scene->normals);
}

void toggle_texture()
{
    if (glIsEnabled(GL_TEXTURE_2D) == GL_FALSE) {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
    
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

