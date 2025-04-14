#include "app.h"

#include <stdio.h>

/**
 * Main function
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char* argv[])
{
#pragma GCC diagnostic pop
    App app;

    init_app(&app, 800, 600);
    while (app.is_running) {
        handle_app_events(&app);
        update_app(&app);
        render_app(&app);
    }
    destroy_app(&app);

    return 0;
}
