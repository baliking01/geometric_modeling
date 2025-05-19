#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "texture.h"

#include <obj/model.h>

typedef struct Scene
{
    vec3 *points;
    vertex_t *disp_points;
    double *dz;

    Material material;

    int dim_n;
    int dim_m;
    int res;

    // visibility
    int normals;
    int control_polygon;

    GLuint texture_id;
} Scene;


/**
 * Initialize the scene by loading models.
 */
void init_scene(Scene* scene);

/**
 * Set the lighting of the scene.
 */
void set_lighting();

/**
 * Set the current material.
 */
void set_material(const Material* material);

/**
 * Update the scene.
 */
void update_scene(Scene* scene);

void init_surface(Scene *scene, int dim_n, int dim_m, int res);
void generate_surface(Scene *scene);
void premap_texture(Scene *scene);
void change_dim(Scene *scene, int target_dim, int size);

void toggle_control_polygon(Scene *scene);
void toggle_normals(Scene *scene);
void toggle_texture();

/**
 * Render the scene objects.
 */
void render_scene(const Scene* scene);

/**
 * Draw the origin of the world coordinate system.
 */
void draw_origin();

#endif /* SCENE_H */
