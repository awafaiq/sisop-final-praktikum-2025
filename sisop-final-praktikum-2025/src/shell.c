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
  int i = 0; 
  int j = 0;
  int k = 0;

  // Kosongkan semua
  clear(cmd, 64);
  clear(arg[0], 64);
  clear(arg[1], 64);

  // Salin cmd
  while (buf[i] != ' ' && buf[i] != '\0') {
    cmd[i] = buf[i];
    i++;
  }
  cmd[i] = '\0';

  if (buf[i] == ' ') i++; 
  
  // Argumen pertama
  while (buf[i] != ' ' && buf[i] != '\0') {
    arg[0][j++] = buf[i++];
  }
  arg[0][j] = '\0';

  if (buf[i] == ' ') i++; 

  // Argumen kedua
  while (buf[i] != '\0') {
    arg[1][k++] = buf[i++];
  }
  arg[1][k] = '\0';
}


// TODO: 6. Implement cd function
void cd(byte* cwd, char* dirname) {
struct node_fs node_fs_buf;
struct node_item* node;
int i;
bool found = false;

readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

// cd ..
if (strcmp(dirname, ".."))
{
  if (*cwd != FS_NODE_P_ROOT)
  {
    *cwd = node_fs_buf.nodes[*cwd].parent_index;
    found = true;
  }
  return;
} 

// cd /
if (strcmp(dirname, "/"))
{
  *cwd = FS_NODE_P_ROOT;
  found = true;
  return;
}

//cd dirname

for (i = 0; i < FS_MAX_NODE; i++)
{
  if (node_fs_buf.nodes[i].parent_index == *cwd && strcmp(node_fs_buf.nodes[i].node_name, dirname))
  {
    if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR)
    {
      *cwd = i;
      found = true;
      return;
    }
  }
}

if (!found)
{
  printString("Directory tidak ditemukan!\n");
  return;
}

}


// TODO: 7. Implement ls function
void ls(byte cwd, char* dir_name) {
  struct node_fs node_fs_buf;
  byte target = cwd;
  int i;

  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

  if (strlen(dir_name) > 0 && strcmp(dir_name, ".") == false) {
    bool found = false;
    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwd &&
          node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR &&
          strcmp(node_fs_buf.nodes[i].node_name, dir_name)) {
        target = i;
        found = true;
        break;
      }
    }
    if (!found) {
      printString("Directory not found\n");
      return;
    }
  }

  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == target) {
      printString(node_fs_buf.nodes[i].node_name);
      printString("\n");
    }
  }
}


// TODO: 8. Implement mv function
void mv(byte cwd, char* src, char* dst) {}

// TODO: 9. Implement cp function
void cp(byte cwd, char* src, char* dst) {}

// TODO: 10. Implement cat function
void cat(byte cwd, char* filename) {}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char* dirname) {}

