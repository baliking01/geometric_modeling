#include <SDL2/SDL.h>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define N_POINTS 4
#define DEGREE 3
#define N_KNOTS N_POINTS + DEGREE

const double POINT_RADIUS = 10.0;
const int INTERP_RES = 100;

/**
 * A simple point structure.
 */
typedef struct Point
{
    double x;
    double y;
} Point;


/*

Normalized B-spline
k: degree
u: position
*/ 
double base_spline(int i, int k, double u, double *knots)
{
    if (k == 1) {
        return (knots[i] <= u && u < knots[i + 1]) ? 1.0 : 0.0;
    }

    double left = 0.0, right = 0.0;

    if (knots[i + k - 1] != knots[i]) {
        left = (u - knots[i]) / (knots[i + k - 1] - knots[i]) * base_spline(i, k-1, u, knots);
    }

    if (knots[i + k] != knots[i + 1]) {
        right = (knots[i + k] - u) / (knots[i + k] - knots[i + 1]) * base_spline(i+1, k-1, u, knots);
    }

    return left + right;
}

Point de_boor(Point *points, double u, double *knots)
{
    Point res = {0};
    double N;
    for (int i = 0; i < N_POINTS; i++) {
        N = base_spline(i, DEGREE, u, knots);
        res.x += N * points[i].x;
        res.y += N * points[i].y;
    }
    return res;
}


void interpolate_curve(Point *interp_points, Point *points, SDL_Renderer *renderer, double *knots) {
    for (int k = 0; k < INTERP_RES; k++) {
        float t = (float)k / (INTERP_RES - 1);
        interp_points[k] = de_boor(points, t, knots);
    }
}

void draw_curve(SDL_Renderer *renderer, Point *interp_points) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    for (int i = 0; i < INTERP_RES - 1; i++) {
        SDL_RenderDrawLine(renderer, interp_points[i].x, interp_points[i].y,
                           interp_points[i + 1].x, interp_points[i + 1].y);
    }
}

/**
 * C/SDL2 framework for experimentation with curves.
 */
int main(int argc, char* argv[])
{
    int error_code;
    SDL_Window* window;
    bool need_run;
    SDL_Event event;
    SDL_Renderer* renderer;

    const int N = 4;

    int mouse_x, mouse_y;
    int i;

    Point* selected_point = NULL;
    Point points[N_POINTS];
    Point interp_points[INTERP_RES];
    points[0].x = 200;
    points[0].y = 200;
    points[1].x = 400;
    points[1].y = 200;
    points[2].x = 300;
    points[2].y = 400;
    points[3].x = 500;
    points[3].y = 400;

    double knots[N_KNOTS];
    knots[0] = 0.0;
    knots[1] = 0.0;
    knots[2] = 0.0;
    knots[3] = 0.5;
    knots[4] = 1.0;
    knots[5] = 1.0;
    knots[6] = 1.6;

    error_code = SDL_Init(SDL_INIT_EVERYTHING);
    if (error_code != 0) {
        printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
        return error_code;
    }

    window = SDL_CreateWindow(
                 "Splines",
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                 800, 600, 0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    need_run = true;
    while (need_run) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                SDL_GetMouseState(&mouse_x, &mouse_y);
                selected_point = NULL;
                for (int i = 0; i < N_POINTS; ++i) {
                    double dx = points[i].x - mouse_x;
                    double dy = points[i].y - mouse_y;
                    double distance = sqrt(dx * dx + dy * dy);
                    if (distance < POINT_RADIUS) {
                        selected_point = points + i;
                    }
                }
                break;
            case SDL_MOUSEMOTION:
                if (selected_point != NULL) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    selected_point->x = mouse_x;
                    selected_point->y = mouse_y;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                selected_point = NULL;
                break;
            case SDL_KEYDOWN:
            case SDL_SCANCODE_Q:
                need_run = false;
                break;
            case SDL_QUIT:
                need_run = false;
                break;
            }
        }


        // TODO: optimize for efficient redraw
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        // Draw the control points
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
        for (int i = 0; i < N_POINTS; ++i) {
            SDL_RenderDrawLine(renderer, points[i].x - POINT_RADIUS, points[i].y, points[i].x + POINT_RADIUS, points[i].y);
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y - POINT_RADIUS, points[i].x, points[i].y + POINT_RADIUS);
        }
        // Draw the segments
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, SDL_ALPHA_OPAQUE);
        for (int i = 1; i < N_POINTS; ++i) {
            SDL_RenderDrawLine(renderer, points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);
        }

        interpolate_curve(interp_points, points, renderer, knots);
        draw_curve(renderer, interp_points);


        // Display the results
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
