/**
 * @file   _dbio.h
 * @author blackball <blackball@blackball-desktop>
 * @date   Mon Oct 17 10:51:06 2011
 * 
 * @brief  binary data io
 * 
 */

#ifndef _MX_BINARY_IO_H_INCLUDED_
#define _MX_BINARY_IO_H_INCLUDED_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct db_header
{
  int magic;     // file identity
  int num;       // number of nodes 
  int len;       // length of one node(fixed)
  int reserved;  //reserved flag
};

struct db_node
{
  int len;
  struct db_node *next;
  void* data;
};
struct db_node* new_node(int len);
void free_node(struct db_node *n);


#define DB_NAME_LEN 64
struct db
{
  struct db_header hdr;   // header of db
  char fname[DB_NAME_LEN];// file name of db
  struct db_node *head;   // head of list
  struct db_node *tail;   // tail of list
  struct db_node *curr;   // current pointer,
                          // only used for indexing
};

struct db* createdb(int magic, const char *fname,
                    int fixed_len);

void writedb(struct db* d);
void fast_writedb(struct db *d);

struct db* opendb(const char *fname);
struct db* fast_opendb(const char *fname);

void closedb(struct db* d);
void fast_closedb(struct db* d);

void dump(struct db* d, struct db_node *n);
struct db_node* undump(struct db *d, int idx);


#ifdef __cplusplus
}
#endif

#endif
