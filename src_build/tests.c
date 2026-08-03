#include "../src_build/config.h"

#define NOB_IMPLEMENTATION
#include "../3rdparty/nob.h"

#define ROM_ASSEMBLER "vasm6502_oldstyle"
int main(int argc, char** argv) {
  NOB_TODO("Build and run tests");

  nob_log(NOB_INFO, "Building tests");
  if (!nob_mkdir_if_not_exists(TESTS_BUILD_FOLDER)) return 1;

  nob_log(NOB_INFO, "Running tests");
  if (!nob_mkdir_if_not_exists(TESTS_BUILD_FOLDER)) return 1;

  return 0;
}
