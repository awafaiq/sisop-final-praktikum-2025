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
  if (cwd == 0xFF) {
    printString("/");
    return;
  }

  struct node_filesystem filesystem;
  readSector(&filesystem, 0x101); // Membaca node part 1
  readSector(((byte*)&filesystem) + SECTOR_SIZE, 0x102); // Membaca node part 2

  char fullpath[128] = "";

  while (cwd != 0xFF) {
    struct node_entry currentNode = filesystem.nodes[cwd];

    //Mengabungkan nama direktori sekarang ke dalam path
    char temp[64];
    strcpy(temp, "/");
    strcat(temp, currentNode.name);
    strcat(temp, fullpath);
    strcpy(fullpath, temp);

    cwd = currentNode.parent_node_index;
  }

  printString(fullpath);
}


// TODO: 5. Implement parseCommand function
void parseCommand(char* buf, char* cmd, char arg[2][64]) {
  int i = 0;
  int j = 0;
  int argIndex = 0;

  while (buf[i] == ' ') i++; // melewati spasi

  while (buf[i] != ' ' && buf[i] != '\0') {
    cmd[j++] = buf[i++];
  }
  cmd[j] = '\0';

  for (argIndex = 0; argIndex < 2; argIndex++) {
    while (buf[i] == ' ') i++; // Melewati spasi antar argumen
    j = 0;

    while (buf[i] != ' ' && buf[i] != '\0') {
      arg[argIndex][j++] = buf[i++];
    }
    arg[argIndex][j] = '\0';
  }
}

// TODO: 6. Implement cd function
void cd(byte* cwd, char* dir_name) {
  struct node_filesystem filesystem;
  readSector(&filesystem, 0x101);
  readSector(((byte*)&filesystem) + SECTOR_SIZE, 0x102);

  // cd / = ke root
  if (strcmp(dir_name, "/") == 0) {
    *cwd = 0xFF;
    return;
  }

  // cd .. = naik satu level
  if (strcmp(dir_name, "..") == 0) {
    if (*cwd != 0xFF) {
      *cwd = filesystem.nodes[*cwd].parent_node_index;
    }
    return;
  }

  // cd <folder> = cari folder di cwd
  for (int i = 0; i < 64; i++) {
    struct node_entry nodes = filesystem.nodes[i];

    if (node.parent_node_index == *cwd &&
        strcmp(node.name, dir_name) == 0 &&
        node.sector_entry_index == FS_NODE_S_IDX_FOLDER) {
      *cwd = i;
      return;
    }
  }

  // Direktori tidak ditemukan
  printString("cd: no such directory\n");
}

// TODO: 7. Implement ls function
void ls(byte cwd, char* dir_name) {
  struct node_filesystem filesystem;
  readSector(&filesystem, 0x101);
  readSector(((byte*)&filesystem) + SECTOR_SIZE, 0x102);

  byte target_dir = cwd;

  // ls <dirname> = ubah target ke direktori tujuan
  if (strcmp(dir_name, ".") != 0 && strlen(dir_name) > 0) {
    boolean found = FALSE;

    for (int i = 0; i < 64; i++) {
      struct node_entry node = filesystem.nodes[i];

      if (node.parent_node_index == cwd &&
          strcmp(node.name, dir_name) == 0 &&
          node.sector_entry_index == FS_NODE_S_IDX_FOLDER) {
        target_dir = i;
        found = TRUE;
        break;
      }
    }
    if (!found) {
      printString("ls: directory not found\n");
      return;
    }
  }

  // Tampilkan semua entri di direktori target
  for (int i = 0; i < 64; i++) {
    struct node_entry node = filesystem.nodes[i];

    if (node.parent_node_index == target_dir && node.name[0] != '\0') {
      printString(node.name);
      if (node.sector_entry_index == FS_NODE_S_IDX_FOLDER)
        printString("/"); // Tandai folder
      printString("  ");
    }
  }

  printString("\n");
}


// TODO: 8. Implement mv function
void mv(byte cwd, char* src, char* dst) {}

// TODO: 9. Implement cp function
void cp(byte cwd, char* src, char* dst) {}

// TODO: 10. Implement cat function
void cat(byte cwd, char* filename) {}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char* dirname) {}

