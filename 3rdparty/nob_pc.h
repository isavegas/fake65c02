/* nob_pc.h - v0.1.0 - Public Domain - https://github.com/isavegas/nob_pc.h

    Addon to nob.h for querying pkg-config.

    # Raylib speedrun
    ```c
      #define NOB_IMPLEMENTATION
      #include "nob.h"
      #define NOB_PC_IMPLEMENTATION
      #include "nob_pc.h"

      int main(int argc, char **argv) {
          NOB_GO_REBUILD_URSELF(argc, argv);
          Nob_Cmd cmd = {0};
          nob_cc(&cmd);
          nob_cc_flags(&cmd);
          if (!nob_cmd_append_pc_cflags(&cmd, "raylib")) return 1;
          nob_cc_inputs(&cmd, "src/game.c");
          nob_cc_output(&cmd, "build/game");
          if (!nob_cmd_append_pc_libs(&cmd, "raylib")) return 1;
          if (!nob_cmd_run(&cmd)) return 1;
          nob_cmd_free(cmd);
      }
    ```
*/

#ifndef NOB_PC_H
#define NOB_PC_H
// Prevent linking errors
#undef NOB_IMPLEMENTATION

#include "nob.h"

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <namedpipeapi.h>
#else
#include <unistd.h>
#endif //_WIN32

#ifndef PKG_CONFIG_EXECUTABLE
#define PKG_CONFIG_EXECUTABLE "pkg-config"
#endif // PKG_CONFIG_EXECUTABLE

#ifndef NOB_CAPTURE_BUFFER_SIZE
// pkg-config won't have massive outputs, so I use a small buffer.
// You'll want to increase this if you'll be capturing lots of data!
#define NOB_CAPTURE_BUFFER_SIZE 32
#endif // NOB_CAPTURE_BUFFER_SIZE

NOBDEF int nob_cmd_run_capture(Nob_Cmd *cmd, char **out);
NOBDEF int nob_pc_found();
NOBDEF int nob_pc_version(char **out);
NOBDEF int nob_pc_exists(char *package);
NOBDEF int nob_pc_cflags(char *package, char **out);
NOBDEF int nob_pc_cflags_I(char *package, char **out);
NOBDEF int nob_pc_cflags_other(char *package, char **out);
NOBDEF int nob_pc_libs(char *package, char **out);
NOBDEF int nob_pc_libs_L(char *package, char **out);
NOBDEF int nob_pc_libs_l(char *package, char **out);
NOBDEF int nob_pc_modversion(char *package, char **out);

NOBDEF int nob_cmd_append_pc_cflags(Nob_Cmd *cmd, char *package);
NOBDEF int nob_cmd_append_pc_cflags_I(Nob_Cmd *cmd, char *package);
NOBDEF int nob_cmd_append_pc_libs(Nob_Cmd *cmd, char *package);

#endif // NOB_PC_H

#ifdef NOB_PC_IMPLEMENTATION
#undef NOB_PC_IMPLEMENTATION

// NOLINTBEGIN(misc-definitions-in-headers)
// TODO: Test this on Windows
NOBDEF int nob_cmd_run_capture(Nob_Cmd *cmd, char **out) {
  bool result = 1;
  Nob_String_Builder cmd_sb = {0};
  nob_cmd_render(*cmd, &cmd_sb);
  cmd->count = 0;
  nob_sb_append_null(&cmd_sb);
  const char *cmd_str = nob_temp_sv_to_cstr(nob_sb_to_sv(cmd_sb));

#if defined _MSC || defined __MINGW32__
  FILE *fd = _popen(cmd_str, "r");
#else
  FILE *fd = popen(cmd_str, "r");
#endif // _MSC || __MINGW32__

  Nob_String_Builder sb = {0};
  char buffer[NOB_CAPTURE_BUFFER_SIZE] = {0};
  if (fd == NULL)
    // clang-tidy gets very unhappy if I initialize any variables nob_return_defer
    nob_return_defer(false);
  while (fgets(buffer, sizeof(buffer), fd) != NULL) {
    nob_sb_append_cstr(&sb, buffer); // NOLINT(bugprone-suspicious-realloc-usage)
  }
  (*out) = (char*)nob_temp_sv_to_cstr(nob_sv_trim_right(nob_sb_to_sv(sb)));
  nob_sb_free(sb);
defer:
#ifdef NOB_PC_LOG
  nob_log(NOB_INFO, "CMD: " SV_Fmt, SV_Arg(nob_sb_to_sv(cmd_sb)));
  nob_log(NOB_INFO, "OUTPUT: %s", *out);
#endif
  if (fd != NULL) {
#if defined _MSC || defined __MINGW32__
    _pclose(fd);
#else
    pclose(fd);
#endif // _MSC || __MINGW32__
  }
  return result;
}

NOBDEF int nob_pc_found() {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--version");
  char* out = NULL;
  int result = nob_cmd_run_capture(&cmd, &out);
  return result;
}

NOBDEF int nob_pc_version(char **out) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--version");
  return nob_cmd_run_capture(&cmd, out);
}

NOBDEF int nob_pc_exists(char *package) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--exists", package);
  return nob_cmd_run(&cmd);
}

NOBDEF int nob_pc_cflags(char *package, char **out) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--cflags",
#ifdef _MSC
                 "--msvc-syntax",
#endif
                 package);
  return nob_cmd_run_capture(&cmd, out);
}

NOBDEF int nob_pc_cflags_I(char *package, char **out) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--cflags-only-I",
#ifdef _MSC
                 "--msvc-syntax",
#endif
                 package);
  return nob_cmd_run_capture(&cmd, out);
}

NOBDEF int nob_pc_cflags_other(char *package, char **out) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--cflags-only-other",
#ifdef _MSC
                 "--msvc-syntax",
#endif
                 package);
  return nob_cmd_run_capture(&cmd, out);
}

NOBDEF int nob_pc_libs(char *package, char **out) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--libs",
#ifdef _MSC
                 "--msvc-syntax",
#endif
                 package);
  return nob_cmd_run_capture(&cmd, out);
}

NOBDEF int nob_pc_libs_L(char *package, char **out) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--libs-only-L",
#ifdef _MSC
                 "--msvc-syntax",
#endif
                 package);
  return nob_cmd_run_capture(&cmd, out);
}

NOBDEF int nob_pc_libs_l(char *package, char **out) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--libs-only-l",
#ifdef _MSC
                 "--msvc-syntax",
#endif
                 package);
  return nob_cmd_run_capture(&cmd, out);
}

NOBDEF int nob_pc_modversion(char *package, char **out) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, PKG_CONFIG_EXECUTABLE, "--modversion", package);
  return nob_cmd_run_capture(&cmd, out);
}

NOBDEF int nob_cmd_append_pc_cflags(Nob_Cmd *cmd, char *package) {
  char* out;
  if (!nob_pc_cflags(package, &out))
    return 0;
  nob_cmd_append(cmd, out);
  return 1;
}

NOBDEF int nob_cmd_append_pc_cflags_I(Nob_Cmd *cmd, char *package) {
  char* out;
  if (!nob_pc_cflags_I(package, &out))
    return 0;
  nob_cmd_append(cmd, out);
  return 1;
}
NOBDEF int nob_cmd_append_pc_cflags_other(Nob_Cmd *cmd, char *package) {
  char* out;
  if (!nob_pc_cflags_other(package, &out))
    return 0;
  nob_cmd_append(cmd, out);
  return 1;
}

NOBDEF int nob_cmd_append_pc_libs(Nob_Cmd *cmd, char *package) {
  char* out;
  if (!nob_pc_libs(package, &out))
    return 0;
  nob_cmd_append(cmd, out);
  return 1;
}

NOBDEF int nob_cmd_append_pc_libs_L(Nob_Cmd *cmd, char *package) {
  char* out;
  if (!nob_pc_libs_L(package, &out))
    return 0;
  nob_cmd_append(cmd, out);
  return 1;
}

NOBDEF int nob_cmd_append_pc_libs_l(Nob_Cmd *cmd, char *package) {
  char* out;
  if (!nob_pc_libs_l(package, &out))
    return 0;
  nob_cmd_append(cmd, out);
  return 1;
}
// NOLINTEND(misc-definitions-in-headers)

#endif // NOB_PC_IMPLEMENTATION

#ifdef NOB_STRIP_PREFIX
#define cmd_run_capture nob_cmd_run_capture

#define pc_found nob_pc_found
#define pc_version nob_pc_version
#define pc_exists nob_pc_exists
#define pc_cflags nob_pc_cflags
#define pc_libs nob_pc_libs
#define pc_modversion nob_pc_modversion

#define cmd_append_pc_cflags nob_cmd_append_pc_cflags
#define cmd_append_pc_cflags_I nob_cmd_append_pc_cflags_I
#define cmd_append_pc_cflags_other nob_cmd_append_pc_cflags_other
#define cmd_append_pc_libs nob_cmd_append_pc_libs
#define cmd_append_pc_libs_L nob_cmd_append_pc_libs_L
#define cmd_append_pc_libs_l nob_cmd_append_pc_libs_l
#endif
