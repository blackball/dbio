/**
 * @file   lasttest.c
 * @author blackball <blackball@blackball-desktop>
 * @date   Mon Oct 17 13:13:01 2011
 * 
 * @brief  test lastdbio
 * 
 * 
 */

#include "lastdbio.h"
#include <string.h>
#include <stdio.h>

void pack_double(struct db *database, double d[], int n)
{
  int len = sizeof(double) * n;
  struct db_node *node = new_node(len);
  memcpy(node->data, d, len);
  dump(database, node);
}

// more consideration on unpack 
int unpack_double(struct db *database, double d[], int n, int idx)
{
	int code = -1;
	int len = sizeof(double) * n;

	if (idx > -1)
	{
		struct db_node *node = undump(database, idx);
		if (node != NULL)
		{
			memcpy(d, node->data, len);
			code = 0;
		}
	}

	return code;
}

int main2(int argc, char *argv[])
{

  int magic = 123;
  struct db *database = createdb(magic, "lastest.d", sizeof(double)*5);
  double d[5] = {2,3,4,5,1};
  double s[5] = {1,2,3,4,5};
  pack_double(database, d, 5);
  pack_double(database, s, 5);

  writedb(database);
  
  return 0;
}

int main(int argc ,char *argv[])
{

  struct db * database = opendb("lastest.d");
  double d[5];
  if(unpack_double(database, d, 5, 1))
  {
	  printf("invalid index!\n");
  }
  else
  for(int i = 0; i < 5; ++i)
	  printf("%lf\n", d[i]);

  closedb(database);

  getchar();
  return 0;
}
