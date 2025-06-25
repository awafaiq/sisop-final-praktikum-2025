#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void fsInit() {
  struct map_fs map_fs_buf;
  int i = 0;

  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  for (i = 0; i < 16; i++) map_fs_buf.is_used[i] = true;
  for (i = 256; i < 512; i++) map_fs_buf.is_used[i] = true;
  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
}

// TODO: 2. Implement fsRead function
void fsRead(struct file_metadata* metadata, enum fs_return* status) {
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;

  int i, temp;
  int node_mark = -1;
  bool found = false;
  //1
  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

  //2
  for (i = 0; i < FS_MAX_NODE; i++)
  {
    if (node_fs_buf.nodes[i].parent_index == metadata->parent_index)
    {
      if (strcmp(node_fs_buf.nodes[i].node_name, metadata-> node_name) == 0)
      {
        found = true;
        node_mark = i;
        break;
      } 
    }
  }
  
  //3
  if (!found)
  {
    *status = FS_R_NODE_NOT_FOUND;
    return;
  }

  //4
  if (node_fs_buf.nodes[node_mark].data_index == FS_NODE_D_DIR)
  {
    *status = FS_R_TYPE_IS_DIRECTORY;
    return;
  }

  //5
  metadata->filesize = 0; //a
  temp = node_fs_buf.nodes[node_mark].data_index;
  
  for (i = 0; i < FS_MAX_SECTOR; i++) //b
  {
    if (data_fs_buf.datas[temp].sectors[i] == 0x00)
    {
      break; //c
    }
    readSector(metadata->buffer + i * SECTOR_SIZE, data_fs_buf.datas[temp].sectors[i]); //d
    metadata->filesize += SECTOR_SIZE; //e
  }
  
  //6
  *status = FS_R_SUCCESS;
  
}

// TODO: 3. Implement fsWrite function
void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;

  int i, j;
  int empty_node_mark = -1;
  int empty_data_mark = -1;
  int empty_blocks = 0;

   //1
  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);

  //2
  for (i = 0; i < FS_MAX_NODE; i++)
  {
    if (node_fs_buf.nodes[i].parent_index == metadata->parent_index)
    {
      if (strcmp(node_fs_buf.nodes[i].node_name, metadata-> node_name) == 0)
      {
        *status = FS_W_NODE_ALREADY_EXISTS;
        return;
      } 
    }
  }

  //3
  for (i = 0; i < FS_MAX_NODE; i++) 
  {
    if (node_fs_buf.nodes[i].node_name[0] == '\0') 
    {
        empty_node_mark = i;
        break;
    }
  }

  if (empty_node_mark == -1) 
  {
      *status = FS_W_NO_FREE_NODE;
      return;
  }

  //4
  for (i = 0; i < FS_MAX_DATA; i++)
  {
    if (data_fs_buf.datas[i].sectors[0] == 0x00)
    {
      empty_data_mark = i;
      break;
    }
  }

  if (empty_data_mark == -1)
  {
    *status = FS_W_NO_FREE_DATA;
    return;
  }
  
  //5
  for (i = 0; i < SECTOR_SIZE; i++)
  {
    if (map_fs_buf.is_used[i] == 0x00)
    {
      empty_blocks++;
    }
  }

  if (empty_blocks < metadata->filesize / SECTOR_SIZE)
  {
    *status = FS_W_NOT_ENOUGH_SPACE;
    return;
  }

  //6
  strcpy(node_fs_buf.nodes[empty_node_mark].node_name, metadata->node_name);
  node_fs_buf.nodes[empty_node_mark].parent_index = metadata->parent_index;
  node_fs_buf.nodes[empty_node_mark].data_index = empty_data_mark;  

  //7
  j = 0; //a
  for (i = 0; i < SECTOR_SIZE; i++) //b
  {
    if (map_fs_buf.is_used[i] = false)
    {
      data_fs_buf.datas[empty_data_mark].sectors[j] = i;
      writeSector(metadata->buffer + j * SECTOR_SIZE, i); //c & d
      j++; //e
    }
  }

  //8
  writeSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
  writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);

  //9
  *status = FS_W_SUCCESS;

}
