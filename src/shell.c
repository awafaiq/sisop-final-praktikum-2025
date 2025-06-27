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
    printString("/");
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
void ls(byte cwd, char* dirname) {
  struct node_fs node_fs_buf;
  byte target = cwd;
  int i;

  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

  if (strlen(dirname) > 0 && !(strlen(dirname) == 1 && dirname[0] == '.')) {
    bool found = false;
    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwd &&
          strcmp(node_fs_buf.nodes[i].node_name, dirname)) {
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
    static struct node_fs node_fs_buf;
    static char new_filename[MAX_FILENAME];
    static char target_dirname[MAX_FILENAME];
    int i;
    int src_node_idx = -1;
    int slash = -1;
    int target_dir = -1;
    byte target_parent_idx;
    
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
        printString("File isn't found.\n");
        return;
    }

    // check if src is file, not dir
    if (node_fs_buf.nodes[src_node_idx].data_index == FS_NODE_D_DIR) {
        printString("mv is only for file.\n");
        return;
    }

    // mv <filename> ../<outputname>
    if (strlen(dst) >= 3 && dst[0] == '.' && dst[1] == '.' && dst[2] == '/'){
      if (cwd == FS_NODE_P_ROOT) { printString("Cannot leave from root.\n");
        return;
      }
      target_parent_idx = node_fs_buf.nodes[cwd].parent_index;
      strcpy(new_filename, dst + 3);
    }

    // mv <filename> /<outputname>
    else if (dst[0] == '/'){
        for(i = 1; i < strlen(dst); i++) { if (dst[i] == '/'){
          printString("Absolute path move not supported.\n");
          return;}
        }
        target_parent_idx = FS_NODE_P_ROOT;
        strcpy(new_filename, dst + 1);
    }
    else {
        for(i = 0; i < strlen(dst); i++) { if (dst[i] == '/'){
          slash = i;
          break;
        }
      }

        // if "/", it's <dirname>/<outputname> case
        if (slash != -1){
            for (i = 0; i < slash; i++){
              target_dirname[i] = dst[i];
            }
            target_dirname[slash] = '\0';
            strcpy(new_filename, dst + slash + 1);

            for (i = 0; i < FS_MAX_NODE; i++){
                if (node_fs_buf.nodes[i].parent_index == cwd && node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR && strcmp(node_fs_buf.nodes[i].node_name, target_dirname)){
                    target_dir = i;
                    break;
                }
            }
            if (target_dir == -1) { printString("Target directory isn't found.\n");
              return;
            }
            target_parent_idx = target_dir;
        }
        else {
            for (i = 0; i < FS_MAX_NODE; i++) {
                if (node_fs_buf.nodes[i].parent_index == cwd && node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR && strcmp(node_fs_buf.nodes[i].node_name, dst)){
                    target_dir = i;
                    break;
                }
            }

            // if dst is a dir, just put it there
            if (target_dir != -1){
                target_parent_idx = target_dir;
                strcpy(new_filename, src);
            }
            // if not, rename plz
            else {
                target_parent_idx = cwd;
                strcpy(new_filename, dst);
            }
        }
    }
    
    // Checking if filename is empty
    if (strlen(new_filename) == 0){
        printString("Filename target must be fulfilled.\n");
        return;
    }

    // Checking if filename is already exist
    for (i = 0; i < FS_MAX_NODE; i++){
        if (node_fs_buf.nodes[i].parent_index == target_parent_idx && strcmp(node_fs_buf.nodes[i].node_name, new_filename)){
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
  static struct node_fs node_fs_buf;
  static struct data_fs data_fs_buf;
  static byte map_buf[SECTOR_SIZE];
  static char new_filename[MAX_FILENAME];
  static char content_buf[SECTOR_SIZE];
  static char target_dirname[MAX_FILENAME];

  int i, j;
  int free_sector;
  int src_node_idx = -1;
  int empty_node = -1;
  int empty_data = -1;
  int slash = -1;
  int target_dir = -1;
  byte target_parent_idx;
  byte src_data_entry;
  byte src_content_sector;

    // read
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
    readSector(map_buf, FS_MAP_SECTOR_NUMBER);
    readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
    

    // check file is exist
    for (i = 0; i < FS_MAX_NODE; i++){
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, src)){
            src_node_idx = i;
            break;
        }
    }

    if (src_node_idx == -1){
        printString("File isn't found.\n");
        return;
    }
    if (node_fs_buf.nodes[src_node_idx].data_index == FS_NODE_D_DIR){
        printString("cp is only for file.\n");
        return;
    }

    for(i = 0; i < strlen(dst); i++){
      if (dst[i] == '/'){ 
        slash = i;
        break;
      }
    }

    // parent dir (../<outputname>)
    if (strlen(dst) >= 3 && dst[0] == '.' && dst[1] == '.' && dst[2] == '/'){
        if (cwd == FS_NODE_P_ROOT){
          printString("Cannot leave from root.\n");
          return;
        }
        target_parent_idx = node_fs_buf.nodes[cwd].parent_index;
        strcpy(new_filename, dst + 3);
    }
    // target is filename on cwd
    else if (slash == -1){
        target_parent_idx = cwd;
        strcpy(new_filename, dst);
    }

    // target is <dirname>/<outputname>
    else if (slash > 0) { // if there's /
        // copy name of target dir
        for (i = 0; i < slash; i++) {
            target_dirname[i] = dst[i];
        }
        target_dirname[slash] = '\0';

        strcpy(new_filename, dst + slash + 1);

        // look node for target dir in cwd
        for (i = 0; i < FS_MAX_NODE; i++) {
            if (node_fs_buf.nodes[i].parent_index == cwd && node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR && strcmp(node_fs_buf.nodes[i].node_name, target_dirname)){
                target_dir = i;
                break;
            }
        }
        if (target_dir == -1){
          printString("Target directory isn't found.\n");
          return;
        }
        target_parent_idx = target_dir;
    }

    // root dir /<outputname>
    else if (slash == 0){
        for(i = 1; i < strlen(dst); i++){
             if (dst[i] == '/') {
                printString("Absolute path is not supported.\n");
                return;
             }
        }
        target_parent_idx = FS_NODE_P_ROOT;
        strcpy(new_filename, dst + 1);
    }
    else{
        printString("cp format isn't valid.\n");
        return;
    }

    if (strlen(new_filename) == 0){
      printString("Filename target must be fulfilled.\n");
      return;
    }


    for (i = 0; i < FS_MAX_NODE; i++){
      if (node_fs_buf.nodes[i].node_name[0] == '\0'){
        empty_node = i;
        break;
      }
    }
    for (i = 0; i < FS_MAX_DATA; i++){
      if (data_fs_buf.datas[i].sectors[0] == 0x00){
        empty_data = i;
        break;
      }
    }
    
    // validation
    if (empty_node == -1){
      printString("Not enough node space.\n");
      return;
    }
    if (empty_data == -1){
      printString("Not enough data entry space.\n");
      return;
    }

    for (i = 0; i < FS_MAX_NODE; i++){
        if (node_fs_buf.nodes[i].parent_index == target_parent_idx && strcmp(node_fs_buf.nodes[i].node_name, new_filename)){
            printString("Filename is on target.\n");
            return;
        }
    }
    
    // link new node
    node_fs_buf.nodes[empty_node].data_index = empty_data;
    strcpy(node_fs_buf.nodes[empty_node].node_name, new_filename);
    node_fs_buf.nodes[empty_node].parent_index = target_parent_idx;

    // copy its file content
    src_data_entry = node_fs_buf.nodes[src_node_idx].data_index;
    for (j = 0; j < FS_MAX_SECTOR; j++){
        src_content_sector = data_fs_buf.datas[src_data_entry].sectors[j];
        if (src_content_sector == 0x00){
          break;
        }
        
        // check if sector is empty
        free_sector = -1;
        for (i = 0x10; i < 0x100; i++){ //0x10 - 0xFF
            if (map_buf[i] == 0x00){
                free_sector = i;
                map_buf[i] = 0x01;
                break;
            }
        }
        if (free_sector == -1){
            printString("Not enough space on disk.\n");
            return;
        }
        data_fs_buf.datas[empty_data].sectors[j] = free_sector;
        
        //rw
        readSector(content_buf, src_content_sector);
        writeSector(content_buf, free_sector);
    }
    

    // write
    writeSector(map_buf, FS_MAP_SECTOR_NUMBER);
    writeSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
    writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
}

// TODO: 10. Implement cat function
void cat(byte cwd, char* filename) {
    struct node_fs node_fs_buf;
    struct data_fs data_fs_buf;
    char content_buf[SECTOR_SIZE];
    int i, j; int file_node_idx = -1; //set null
    byte data_entry;
    byte content_sector;

    // read 
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

    // search file in cwd
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, filename)){
            file_node_idx = i;
            break;
        }
    }

    if (file_node_idx == -1){
      printString("File isn't found.\n");
      return;
    }

    if (node_fs_buf.nodes[file_node_idx].data_index == FS_NODE_D_DIR){
      printString("cat is only for file.\n");
      return;
    }

    readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
    data_entry = node_fs_buf.nodes[file_node_idx].data_index;


    for (j = 0; j < FS_MAX_SECTOR; j++){
        content_sector = data_fs_buf.datas[data_entry].sectors[j];
        if (content_sector == 0x00){
            break;
        }
        // read n print
        readSector(content_buf, content_sector);
        printString(content_buf);
    }
    printString("\n");
}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char* dirname) {
    struct node_fs node_fs_buf;
    int i; int empty_node = -1; //set null
    bool same_name = false;

    // read
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

    for (i = 0; i < FS_MAX_NODE; i++){
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, dirname)){
            same_name = true;
            break;
        }
        if (node_fs_buf.nodes[i].node_name[0] == '\0' && empty_node == -1){
            empty_node = i;
        }
    }

    // error handling if same name exists
    if (same_name){
        printString("Directory or file with that name already exists.\n");
        return;
    }

    // if empty, create new dir
    if (empty_node != -1){
        // fill new data
        node_fs_buf.nodes[empty_node].parent_index = cwd;
        strcpy(node_fs_buf.nodes[empty_node].node_name, dirname);
        node_fs_buf.nodes[empty_node].data_index = FS_NODE_D_DIR;

        // rewrite
        writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
        writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
    }
    else {
        // error handling if full
        printString("File system is full, cannot create directory.\n");
    }
}
