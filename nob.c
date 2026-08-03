#include "./src_build/config.h"

#define NOB_PC_IMPLEMENTATION
#include "./3rdparty/nob_pc.h"

#define NOB_IMPLEMENTATION
#include "./3rdparty/nob.h"

// https://en.wikipedia.org/wiki/X_macro
#define FAKE65C02_SOURCES \
    X(SRC_FOLDER"main.c") \
    X(SRC_FOLDER"machine.c") \
    X(SRC_FOLDER"fake65c02.c")

#ifdef DEBUG
#define ADD_CFLAGS(cmd) nob_cmd_append((cmd), "-g", "-fsanitize=address,undefined", "-O0")
#else
#define ADD_CFLAGS(cmd) nob_cmd_append((cmd), "-march=native", "-mtune=native", "-O2", "-flto=thin")
#endif // DEBUG

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF_PLUS(argc, argv, "3rdparty/nob.h", "./3rdparty/nob_pc.h", "src_build/config.h");
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Nob_Cmd cmd = {0};

    // ------------------------------------------------------------------------------------
    // fake65c02
    nob_cmd_append(&cmd, CC, "-Wall", "-Wextra", "-I"INCLUDES, "-I"THIRD_PARTY, "-o", BUILD_FOLDER"fake65c02");
    ADD_CFLAGS(&cmd);
    #define X(source_path) nob_cmd_append(&cmd, source_path);
        FAKE65C02_SOURCES
    #undef X
    if (!nob_cmd_run(&cmd)) return 1;

    // ------------------------------------------------------------------------------------
    // roms
    cmd_append(&cmd, NOB_REBUILD_URSELF(BUILD_FOLDER "build_roms", "./src_build/build_roms.c")); // NOLINT
    if (!nob_cmd_run(&cmd)) return 1; // build rom builder
    cmd_append(&cmd, BUILD_FOLDER "build_roms");
    if (!nob_cmd_run(&cmd)) return 1; // run rom builder

    // ------------------------------------------------------------------------------------
    // tests
    /*
    cmd_append(&cmd, NOB_REBUILD_URSELF(BUILD_FOLDER "tests", "./src_build/tests.c"));
    if (!nob_cmd_run(&cmd)) return 1; // build tests harness
    cmd_append(&cmd, BUILD_FOLDER "tests");
    if (!nob_cmd_run(&cmd)) return 1; // run tests harness
    */
    nob_cmd_free(cmd);
    return 0;
}
