#ifndef CONFIG_H
#define CONFIG_H

#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"
#define THIRD_PARTY  "3rdparty/"
#define INCLUDES     "include/"
#define ROMS_FOLDER "roms/"
#define ROMS_BUILD_FOLDER BUILD_FOLDER ROMS_FOLDER

#define TESTS_FOLDER "tests/"
#define TESTS_BUILD_FOLDER BUILD_FOLDER TESTS_FOLDER

#ifdef __TINYC__
#define CC "tcc"
#elif defined _MSC || defined __MINGW32__
#define CC "musl-clang"
#else
#define CC "clang"
#endif

#define NOB_REBUILD_URSELF(binary_path, source_path) "clang", "-x", "c", "-g", "-O0", "-o", binary_path, source_path

#endif // CONFIG_H