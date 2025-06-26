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

  // Jika nama direktori diberikan dan bukan "."
  if (strlen(dir_name) > 0 && !(strlen(dir_name) == 1 && dir_name[0] == '.')) {
    bool found = false;

    // Cari node direktori dengan nama tersebut di bawah cwd
    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwd &&
          strcmp(node_fs_buf.nodes[i].node_name, dir_name)) {
        if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
          target = i;
          found = true;
          break;
        }
      }
    }

    if (!found) {
      printString("Directory not found\n");
      return;
    }
  }
  
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == target &&
        strlen(node_fs_buf.nodes[i].node_name) > 0) {
      printString(node_fs_buf.nodes[i].node_name);
      printString("\n");
    }
  }
}

// TODO: 8. Implement mv function
void mv(byte cwd, char* src, char* dst) {
    struct node_fs node_fs_buf;
    int i; int src_node_idx = -1; //set null
    byte target_parent_idx;
    char new_filename[MAX_FILENAME]; 
    
    // read
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

    // search src in cwd
    for (i = 0; i < FS_MAX_NODE; i++){
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, src)){
            src_node_idx = i;
            break;
        }
    }

    //if not to be found
    if (src_node_idx == -1){
        printString("File isn't found!\n");
        return;
    }

    // check if src is file, not dir
    if (node_fs_buf.nodes[src_node_idx].data_index == FS_NODE_D_DIR) {
        printString("mv is only for file\n");
        return;
    }

    // Case 1: mv <filename> ../<outputname>
    if (strncmp(dst, "../", 3)){
        if (cwd == FS_NODE_P_ROOT) {
            printString("Cannot leave from root.\n");
            return;
        }
        target_parent_idx = node_fs_buf.nodes[cwd].parent_index;
        strcpy(new_filename, dst + 3); // copy its name after "../"
    }

    // Case 2: mv <filename> /<outputname>
    else if (strncmp(dst, "/", 1)){
        target_parent_idx = FS_NODE_P_ROOT;
        strcpy(new_filename, dst + 1); // copy its name after "/"
    }

    // Case 3: mv <filename> <dirname>/<outputname>
    else{
        int slash_idx = -1;
        int target_dir_node_idx = -1;
        char target_dirname[MAX_FILENAME]; // variabel for saving filename target

        // Loop for checking '/'
        for(i = 0; i < strlen(dst); i++) {
            if (dst[i] == '/') {
                slash_idx = i;
                break;
            }
        }
        
        if (slash_idx != -1) {
            
            // get target directory name
            strncpy(target_dirname, dst, slash_idx);
            target_dirname[slash_idx] = '\0';

            // copy its name after "/"
            strcpy(new_filename, dst + slash_idx + 1);

            //check if target directory is exist
            for (i = 0; i < FS_MAX_NODE; i++){
                if (node_fs_buf.nodes[i].parent_index == cwd && 
                    node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR && 
                    strcmp(node_fs_buf.nodes[i].node_name, target_dirname)) {
                    target_dir_node_idx = i;
                    break;
                }
            }

            // if target directory isn't found
            if (target_dir_node_idx == -1){
                printString("Target directory isn't found.\n");
                return;
            }
            target_parent_idx = target_dir_node_idx;

        }
        else{
            printString("mv format isn't valid.\n");
            return;
        }
    }
    
    // Checking if filename is empty
    if (strlen(new_filename) == 0){
        printString("Filename target must be fulfilled.\n");
        return;
    }

    // Checking if filename is already exist
    for (i = 0; i < FS_MAX_NODE; i++){
        if (node_fs_buf.nodes[i].parent_index == target_parent_idx && strcmp(node_fs_buf.nodes[i].node_name, new_filename)) {
            printString("Filename is on target.\n");
            return;
        }
    }

    // change parent_index and node_name
    node_fs_buf.nodes[src_node_idx].parent_index = target_parent_idx;
    strcpy(node_fs_buf.nodes[src_node_idx].node_name, new_filename);

    // Rewrite 
    writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
}

// TODO: 9. Implement cp function
void cp(byte cwd, char* src, char* dst) {
    struct node_fs node_fs_buf;
    int i;
    int src_node_idx = -1;
    byte target_parent_idx;
    char new_filename[MAX_FILENAME];

    // 1. read node sectors
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

    // 2. search src in cwd
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, src)){
            src_node_idx = i;
            break;
        }
    }

    if (src_node_idx == -1) {
        printString("File isn't found!\n");
        return;
    }

    // 3. check if src is file, not dir
    if (node_fs_buf.nodes[src_node_idx].data_index == FS_NODE_D_DIR){
        printString("cp is only for file!\n");
        return;
    }

    // copy from mv
    if (strncmp(dst, "../", 3)){
        if (cwd == FS_NODE_P_ROOT){
          printString("Cannot leave from root.\n");
          return;
        }
        target_parent_idx = node_fs_buf.nodes[cwd].parent_index;
        strcpy(new_filename, dst + 3);
    }
    else if (strncmp(dst, "/", 1)){
        target_parent_idx = FS_NODE_P_ROOT;
        strcpy(new_filename, dst + 1);
    }
    else {
        int slash_idx = -1, target_dir_node_idx = -1;
        char target_dirname[MAX_FILENAME];
        for(i = 0; i < strlen(dst); i++){
          if (dst[i] == '/'){
            slash_idx = i;
            break;
          }
        }
        
        if (slash_idx != -1){
            strncpy(target_dirname, dst, slash_idx);
            target_dirname[slash_idx] = '\0';
            strcpy(new_filename, dst + slash_idx + 1);
            for (i = 0; i < FS_MAX_NODE; i++){
                if (node_fs_buf.nodes[i].parent_index == cwd && node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR && strcmp(node_fs_buf.nodes[i].node_name, target_dirname)){
                    target_dir_node_idx = i;
                    break;
                }
            }
            if (target_dir_node_idx == -1){
              printString("Target directory isn't found.\n");
              return;
            }
            target_parent_idx = target_dir_node_idx;
        }
        else {
          printString("cp format isn't valid.\n");
          return;
        }
    }

    if (strlen(new_filename) == 0) { printString("Filename target must be fulfilled.\n"); return; }

    // 5. Find empty resources (node and data sector)
    int empty_node_idx = -1;
    int empty_sector_idx = -1;
    bool sector_is_used[256]; // Menggunakan array lokal, bukan dari disk

    // Initialize all sectors as free
    for (i = 0; i < 256; i++) {
        sector_is_used[i] = false;
    }

    // Mark system sectors as used
    sector_is_used[FS_NODE_SECTOR_NUMBER] = true;
    sector_is_used[FS_NODE_SECTOR_NUMBER + 1] = true;

    // Mark used data sectors by checking all file nodes
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].node_name[0] != '\0' && node_fs_buf.nodes[i].data_index != FS_NODE_D_DIR) {
            sector_is_used[node_fs_buf.nodes[i].data_index] = true;
        }
        // find an empty node
        if (node_fs_buf.nodes[i].node_name[0] == '\0' && empty_node_idx == -1) {
            empty_node_idx = i;
        }
    }

    // Find the first free sector
    for (i = 0; i < 256; i++) {
        if (sector_is_used[i] == false) {
            empty_sector_idx = i;
            break;
        }
    }

    if (empty_node_idx == -1 || empty_sector_idx == -1){
        printString("Not enough space on disk or filesystem.\n");
        return;
    }

    // Check if filename already on target
    for (i = 0; i < FS_MAX_NODE; i++){
        if (node_fs_buf.nodes[i].parent_index == target_parent_idx && strcmp(node_fs_buf.nodes[i].node_name, new_filename)){
            printString("Filename is on target.\n");
            return;
        }
    }

    // copy content
    char content_buf[SECTOR_SIZE];
    readSector(content_buf, node_fs_buf.nodes[src_node_idx].data_index);
    writeSector(content_buf, empty_sector_idx);

    // update node
    node_fs_buf.nodes[empty_node_idx].parent_index = target_parent_idx;
    strcpy(node_fs_buf.nodes[empty_node_idx].node_name, new_filename);
    node_fs_buf.nodes[empty_node_idx].data_index = empty_sector_idx;

    // Rewrite
    writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
}

// TODO: 10. Implement cat function
void cat(byte cwd, char* filename) {}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char* dirname) {}

