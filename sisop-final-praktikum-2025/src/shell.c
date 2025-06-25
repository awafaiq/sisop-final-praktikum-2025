#include "shell.h"
#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void shell() {
  char buf[64];
  char cmd[64];
  char arg[2][64];

  byte cwd = FS_NODE_P_ROOT;

  while (true) {
    printString("MengOS:");
    printCWD(cwd);
    printString("$ ");
    readString(buf);
    parseCommand(buf, cmd, arg);

    if (strcmp(cmd, "cd")) cd(&cwd, arg[0]);
    else if (strcmp(cmd, "ls")) ls(cwd, arg[0]);
    else if (strcmp(cmd, "mv")) mv(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cp")) cp(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cat")) cat(cwd, arg[0]);
    else if (strcmp(cmd, "mkdir")) mkdir(cwd, arg[0]);
    else if (strcmp(cmd, "clear")) clearScreen();
    else printString("Invalid command\n");
  }
}

// TODO: 4. Implement printCWD function
void printCWD(byte cwd) {
  struct node_fs node_fs_buf;
  int i, j;
  int depth = 0;
  char path[FS_MAX_NODE][MAX_FILENAME];
  byte temp;

  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

  //jika cwd root
  if (cwd == FS_NODE_P_ROOT)
  {
    printString("/");
    return;
  }

  //cari path root ke cwd
  while (cwd != FS_NODE_P_ROOT)
  {
    temp = cwd;
    strcpy(path[depth], node_fs_buf.nodes[temp].node_name);
    cwd = node_fs_buf.nodes[temp].parent_index;
    depth++;
  }

  printString("/");

  //print pathnya
  for (j = depth - 1; j >= 0; j--)
  {
    printString(path[j]);
    if (j == 0)
    {
      printString("/");
    }
  }
}


// TODO: 5. Implement parseCommand function
void parseCommand(char* buf, char* cmd, char arg[2][64]) {
}

// TODO: 6. Implement cd function
void cd(byte* cwd, char* dir_name) {

}

// TODO: 7. Implement ls function
void ls(byte cwd, char* dir_name) {

}


// TODO: 8. Implement mv function
void mv(byte cwd, char* src, char* dst) {}

// TODO: 9. Implement cp function
void cp(byte cwd, char* src, char* dst) {}

// TODO: 10. Implement cat function
void cat(byte cwd, char* filename) {}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char* dirname) {}

