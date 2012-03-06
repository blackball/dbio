/**
 * @file   dbio.cpp
 * @author blackball <blackball@blackball-desktop>
 * @date   Mon Oct 17 16:08:13 2011
 * 
 * @brief  definitions of user defined pack/unpack methods
 * 
 * 
 */

#include "dbio.h"
#include "_dbio.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * pack any type of array 
 * 
 * @param dbase database 
 * @param data data chunk
 * @param len sizeof(type)*n
 * 
 * @return error code, 0 means OK, -1 means failed
 */
int pack_arr(struct db *dbase, void *data, int len)
{
  int code = -1;

  struct db_node *node = new_node(len);
  memcpy(node->data, data, len);
  // insert node into dbase
  dump(dbase, node);
  
  code = 0;
  return code;
}

  /** 
   * unpack a array data from database
   * 
   * @param dbase database handle
   * @param data array 
   * @param len sizeof(type)*n
   * @param idx -1 for continious, [0, len-1] for idx
   * 
   * @return 
   */
int unpack_arr(struct db* dbase, void *data, int len, int idx)
{
  int code = -1;
  struct db_node *node = undump(dbase, idx);
  if (node != NULL)
  {
    memcpy(data,node->data,len);
    code = 0;
  }
  return code;
}


#ifdef __cplusplus
}
#endif
