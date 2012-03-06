/**
 * @file   dbio.h
 * @author blackball <blackball@blackball-desktop>
 * @date   Mon Oct 17 16:05:47 2011
 * 
 * @brief  user defined IO functions for database 
 * operations. define your pack/unpack methods here.
 * 
 */

#ifndef _MX_DB_IO_H_INCLUDED_
#define _MX_DB_IO_H_INCLUDED_

#include "_dbio.h"

#ifdef __cplusplus
extern "C" {
#endif

int pack_arr(struct db *dbase, void *data, int len);
int unpack_arr(struct db *dbase, void *data, int len, int idx);

#ifdef __cplusplus
}
#endif


#endif
