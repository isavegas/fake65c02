#include "../src_build/config.h"

#define NOB_IMPLEMENTATION
#include "../3rdparty/nob.h"

#define ROM_ASSEMBLER "vasm6502_oldstyle"
int main(int argc, char** argv) {
  nob_log(NOB_INFO, "Building roms");
  if (!nob_mkdir_if_not_exists(ROMS_BUILD_FOLDER)) return 1;

  Nob_File_Paths children = {0};
  nob_read_entire_dir(ROMS_FOLDER, &children);

  // TODO: walk dirs
  #ifdef DONT_DO
  if (children.count <= 2) {
    nob_log(NOB_INFO, "No roms to build");
  } else {
    for (int i = 0; i < children.count; i++) {
      const char* name = children.items[i];
      if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
        char src_path[128] = {0};
        char out_path[128] = {0};
        if (sprintf(src_path, "%s/%s", ROMS_FOLDER, name) == 0) return 1;
        if (sprintf(out_path, "%s/%s", ROMS_BUILD_FOLDER, name) == 0) return 1;
        nob_log(NOB_INFO, "Rom to build: %s", src_path);
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, ROM_ASSEMBLER, "-esc", "-c02", "-wdc02", "-quiet",
                       "-chklabels", "-Fbin", "-o", out_path, src_path);
        if (!nob_cmd_run(&cmd)) return 1;
        nob_log(NOB_INFO, "Built: %s", out_path);
      }
    }
  }
  #endif // DONT_DO
  return 0;
}
