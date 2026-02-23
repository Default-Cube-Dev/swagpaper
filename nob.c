#define NOB_IMPLEMENTATION
#include "./nob.h"

Nob_Cmd cmd = {0};

int main(int argc, char *argv[]) {
    NOB_GO_REBUILD_URSELF(argc, argv);


    nob_cmd_append(&cmd, "clang");
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    nob_cmd_append(&cmd, "-lwayland-client", "-lwayland-egl", "-lEGL", "-lGLESv2", "-lm");
    nob_cmd_append(&cmd, "./src/main.c", "./protocol/wlr-layer-shell-unstable-v1.c", "./protocol/xdg-shell.c");
    nob_cmd_append(&cmd, "-o", "swagpaper");

    if (!nob_cmd_run(&cmd)) {return 1;}
    return 0;
}
