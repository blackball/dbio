/**
 * @file   _dbio.cpp
 * @author blackball <blackball@blackball-desktop>
 * @date   Mon Oct 17 16:30:20 2011
 * 
 * @brief  implementation of binary data IO
 * 
 *
 */


#include "_dbio.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

// hooks for future customizing 
typedef int (db_log_fnc)(const char *fmt, ... );
typedef void* (db_malloc_fnc)(size_t sz);
typedef void* (db_realloc_fnc)(void *ptr, size_t sz);
typedef void (db_free_fnc)(void *ptr);
typedef void (db_exit_fnc)(int status);

struct _db_hook
{
  db_log_fnc *log;
  db_malloc_fnc *malloc;
  db_realloc_fnc *realloc;
  db_free_fnc *free;
  db_exit_fnc *exit;
};

/** 
 * default logging function, print warning,error msg
 * out default tp stderr.
 *
 * @param fmt format string 
 * 
 * @return 0 for now, nothing happend
 */
static int _default_log(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  return 0;
}

// customize by your needs 
struct _db_hook hooks =
{ 
  _default_log,
  malloc,
  realloc,
  free,
  exit
};


/** 
 * check if the database header is valid
 * 
 * @param dhr pointer to database header
 * 
 * @return 0 if OK, else wrong 
 */
static int _check_header(struct db_header* hdr)
{
  int s = -1;

  if ((hdr->num > 0) && (hdr->len > 0) &&
      (hdr->reserved == 0))
    s = 0;
  
  return s;
}


/** 
 * create databse 
 * 
 * @param magic a unique identity for database
 * @param fname file name of database file
 * @param fixed_len len of item in database
 * 
 * @return a new db handle
 */
struct db* createdb(int magic, const char *fname,
                  int fixed_len)
{
  struct db *d = (struct db*)hooks.malloc(sizeof(struct db));
  d->hdr.magic = magic;
  d->hdr.num = 0;
  d->hdr.len = fixed_len;
  d->hdr.reserved = 0;
  d->head = NULL;
  d->tail = NULL;
  d->curr = NULL;
  
  if (DB_NAME_LEN < strlen(fname))
  {
    hooks.log("file name is too long! Use a part of it.\n");
  }
  
  strncpy(d->fname, fname, DB_NAME_LEN - 1);

  return d;
}


/** 
 * free database handle
 * 
 * @param d database handle
 */
static void _free_db(struct db *d)
{
  int n;
  struct db_node *curr, *next;
  
  if (d == NULL)
    return ;

  n = d->hdr.num;
  curr = d->head;
  
  int i = 0;
  for (; i < n; ++i)
  {
    if ( curr == NULL ) break; 

    next = curr->next;
    free_node(curr);
    curr = next;
  }

  if ( i != n )
  {
    hooks.log("unexpected error !\n");
    return ;
  }
  
  free(d);
  d = NULL;
}

/** 
 * write data base into disk
 * 
 * @param d database handle
 */
void writedb(struct db *d)
{
  FILE *f = NULL;
  
  if ((f = fopen(d->fname, "wb")) == NULL)
  {
    hooks.log("failed to open file %s!\n", d->fname);
    return ;
  }

  // simple safety check
  if(d)
  if ( _check_header(&(d->hdr)))
  {
    hooks.log("invalid databse!\n");
    fclose(f);
    return ;
  }
  
  // first write header
  fwrite(&(d->hdr), sizeof(struct db_header), 1, f);

  // write content
  struct db_node *curr = d->head;
  int i = 0;
  int num = d->hdr.num;
  for(; i < num; ++i)
  {
    if (curr == NULL ) break;
    fwrite(curr->data, sizeof(char)*(d->hdr.len), 1,  f);
    curr = curr->next;    
  }

  if (i != num)
  {
    hooks.log("unexpected error happens!\n");
  }

  fclose(f);
  _free_db(d);
}

/** 
 * open an exist database with its name
 * 
 * @param fname name of database
 * 
 * @return opened database handle
 */
struct db* opendb(const char *fname)
{
  FILE *f;
  struct db *d = NULL;

  if ((f = fopen(fname, "r")) == NULL)
  {
    hooks.log("can not open file %s!\n", fname);
    return NULL;
  }

  d = (struct db*)hooks.malloc(sizeof(struct db));
  strncpy(d->fname, fname, DB_NAME_LEN-1);
  d->head = NULL;
  d->tail = NULL;
  d->curr = NULL;
  d->hdr.len = 0;
  d->hdr.magic = 0;
  d->hdr.num = 0;
  d->hdr.reserved = 0;

  // constuct database handle
  if (0 ==
      fread(&(d->hdr), sizeof(struct db_header), 1,
            f))
  {
    hooks.log("invalid database file!\n");
    hooks.free(d);
    d = NULL;
  }

  if (d)
  if ( _check_header(&(d->hdr)) )
  {
    hooks.log("invalid databse header!\n");
    hooks.free(d);
    d = NULL;
  }

  if (d)
  {
    // construct node list
    int cnt = d->hdr.num;
    
    while(!feof(f) && cnt > 0)
    {
      struct db_node *n = new_node(d->hdr.len);
      fread(n->data, sizeof(char)*(d->hdr.len),
            1, f);
      dump(d, n);d->hdr.num--;
      --cnt;
    }

    if (cnt != 0)
    {
      hooks.log("unexpected file end!\n");
      hooks.free(d);
      d = NULL;
    }
    
  }
  fclose(f);
  return d;    
}

/** 
 * close database 
 * 
 * @param d 
 */
void closedb(struct db *d)
{
  _free_db(d);
}

/** 
 * create a new node by specify its lenth
 * 
 * @param len bytes in node's data domain
 * 
 * @return new node with allocated memory
 */
struct db_node* new_node(int len)
{
  struct db_node *n = (struct db_node*)
      hooks.malloc(sizeof(struct db_node)+ sizeof(char)*(len));
  n->len = len;
  n->data =(void*)(n+1);
  return n;
}

/** 
 * free node 
 * 
 * @param n a node 
 */
void free_node(struct db_node* n)
{
  hooks.free((void*)n);
  n = NULL;
}

/** 
 * Macro to add new node at the tail of a list
 * 
 * @param tail tail of list, tail = last node
 * @param add new node
 * 
 */
#define _add_node(tail, add)                    \
  do{                                           \
    tail->next = add;                           \
    tail = add;                                 \
    tail->next = NULL;                          \
  }while(0)


/** 
 * insert a node into database
 * 
 * @param d databse handle
 * @param n new node
 */
void dump(struct db* d, struct db_node *n)
{
  if (!d || !n || (d->hdr.len != n->len))
  {
    hooks.log("invalid database or node !\n");
    return ;
  }
  // insert node into db
  if (d->head == NULL) // first node
  {
    d->head = n;
    d->tail = n;
    d->tail->next = NULL;
  }
  else
  _add_node(d->tail, n);

  d->hdr.num++;
}


/** 
 * index and return the item specified
 * 
 * @param d database 
 * @param idx -1 for accessing one by
 * one, [0,1,..,num-1] for indexing
 * 
 * @return pointer to the specified node 
 */
struct db_node* undump(struct db *d, int idx)
{
  struct db_node *node;

  if(idx == -1)
  {
    if (d->curr == NULL) d->curr = d->head;
    else
    {
      d->curr = d->curr->next;
    }
    node = d->curr;
  }
  else if( (idx >= 0) && (idx < d->hdr.num) )
  {
    d->curr = d->head;
    int i = 0;
    for(; i < idx; ++i)
    {
      d->curr = d->curr->next;
    }
    node = d->curr;
  }
  else
  {
    hooks.log("invalid index!\n");
    node = NULL;
  }
  return node;
}


/** 
 * fast version of open database
 * 
 * @param fname 
 * 
 * @return 
 */
struct db* fast_opendb(const char* fname)
{
  // get the header
  FILE *f;
  struct db *d = NULL;

  if ((f = fopen(fname, "r")) == NULL)
  {
    hooks.log("can not open file %s!\n", fname);
    return NULL;
  }

  d = (struct db*)hooks.malloc(sizeof(struct db));
  strncpy(d->fname, fname, DB_NAME_LEN-1);
  d->head = NULL;
  d->tail = NULL;
  d->curr = NULL;
  d->hdr.len = 0;
  d->hdr.magic = 0;
  d->hdr.num = 0;
  d->hdr.reserved = 0;
  
  // constuct database handle
  if (0 ==
      fread(&(d->hdr), sizeof(struct db_header), 1,
            f))
  {
    hooks.log("invalid database file!\n");
    hooks.free(d);
    d = NULL;
  }

  if (d)
  if ( _check_header(&(d->hdr)) )
  {
    hooks.log("invalid databse header!\n");
    hooks.free(d);
    d = NULL;
  }
  
  // because the len of every node is fixed, we could
  // read all nodes' data once, and link them to a list
  if (d)
  {
    int len_total = d->hdr.num * d->hdr.len;
    // for all nodes
    int node_total = d->hdr.num * sizeof(struct db_node);
    void *nodes = hooks.malloc(node_total);
    void *chunk = hooks.malloc(len_total);
    if (chunk == NULL || nodes == NULL)
    {
      hooks.log("insufficient memory!\n");
      hooks.exit(-1);
    }

    // read all data at once
    fread(chunk, len_total, 1, f);

    // constuct database linked list
    int cnt = d->hdr.num;
    int len = d->hdr.len;

    // init every node
    struct db_node *curr = (struct db_node*)nodes;
    int i = 0;
    for(; i < cnt-1; ++i)
    {
      curr->len = len;
      curr->data = NULL;
      curr->next = (curr + 1);
      curr = curr->next;
    }
    d->head = (struct db_node*)nodes;
    d->tail = curr;
    curr->len = len;
    curr->data = NULL;
    curr->next = NULL;
    d->curr = NULL;

    curr = d->head;
    void *data_head = chunk;
    for(i = 0; i < cnt; ++i)
    {
	  if (curr == NULL) break;

      curr->data = data_head;
      curr = curr->next;
      data_head = (unsigned char*)data_head + len;
    }
	curr = NULL;
	data_head = NULL;
  }
 
  return d;
}

/** 
 * fast version of write the database
 * 
 * @param d database handle
 */
void fast_writedb(struct db *d)
{
  // when you create a new database,
  // you don't know the size of it,
  // so we can't write all stuffs once
  // what we could make it fast is,
  // not free the list, left it to OS
  FILE *f = NULL;
  
  if ((f = fopen(d->fname, "wb")) == NULL)
  {
    hooks.log("failed to open file %s!\n", d->fname);
    return ;
  }

  // simple safety check
  if(!d)
  if ( _check_header(&(d->hdr)))
  {
    hooks.log("invalid databse!\n");
    fclose(f);
    return ;
  }
  
  // first write header
  fwrite(&(d->hdr), sizeof(struct db_header), 1, f);

  // write content
  struct db_node *curr = d->head;
  int i = 0;
  int num = d->hdr.num;
  for(; i < num; ++i)
  {
    if (curr == NULL ) break;
    fwrite(curr->data, sizeof(char)*(d->hdr.len), 1,  f);
    curr = curr->next;    
  }

  if (i != num)
  {
    hooks.log("unexpected error happens!\n");
  }
  fclose(f);
  // no memory free here, leave it to OS
}

/** 
 * fast version of close database
 * 
 * @param d 
 */
void fast_closedb(struct db *d)
{
  // if you use fast_opendb(),
  // then you could use this fast close
  // version.
  hooks.free(d->head->data);
  hooks.free(d->head);  
}

#ifdef __cplusplus
}
#endif
