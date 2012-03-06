/**
 * @file   example.cpp
 * @author blackball <blackball@blackball-desktop>
 * @date   Mon Oct 17 18:17:04 2011
 * 
 * @brief  test dbio
 * 
 */

#include <stdio.h>
#include <windows.h>
#include "dbio.h"
#include "galleryio.h"

int test_pack(void)
{
  int arr[10] = {1,2,3,4,5,4,3,2,1,0};
  int len = sizeof(int) * 10;
  
  struct db *dbase = createdb(123, "data.db", len);
  if ( dbase == NULL )
  {
    fprintf(stderr, "unknow error!\n");
    return 0;
  }
  
  for(int i = 0; i < 1000000; ++i)
    if (0 != pack_arr(dbase, arr, len))
    {
      fprintf(stderr, "unknow error!\n");
      return 0;
    }
  
  writedb(dbase);
  return 0;
}

int test_fast_unpack(void)
{
  int arr[10];
  int len = sizeof(int)*10;
  struct db *dbase = fast_opendb("data.db");
  if (dbase == NULL)
  {
    fprintf(stderr, "can not open database!\n");
    return 0;
  }

  if (0 != unpack_arr(dbase, arr, len, -1))
  {
    fprintf(stderr, "unknow error!\n");
  }
  
  for(int i = 0; i < 10; ++i)
    printf("%d ", arr[i]);

  fast_closedb(dbase);
  return 0;
}

int test_unpack(void)
{
  int arr[10];
  int len = sizeof(int)*10;
  struct db *dbase = opendb("data.db");
  if (dbase == NULL)
  {
    fprintf(stderr, "can not open database!\n");
    return 0;
  }

  if (0 != unpack_arr(dbase, arr, len, 1000))
  {
    fprintf(stderr, "unknow error!\n");
  }
  
  for(int i = 0; i < 10; ++i)
    printf("%d ", arr[i]);

  closedb(dbase);
  return 0;
}

void test_gallery()
{
  // Create a item
  GItem *items[5];
  int j;
  for (j = 0; j < 5; ++j)
    items[j] = new_gitem(4800);
	
  int i = 0;
  for (j = 0; j < 5; ++j)
    for (i = 0; i < 4800; ++i)
    {
      items[j]->feature[i] = i + j;
    }
  items[0]->imgID = 0;
  items[0]->label = 0;
  items[1]->imgID = 1;
  items[1]->label = 0;
  items[2]->imgID = 2;
  items[2]->label = 0;
  items[3]->imgID = 3;
  items[3]->label = 1;
  items[4]->imgID = 4;
  items[4]->label = 1;

  // Create a person
  Person *person0 = new_person(3);
  person0->label = 0;
  person0->items[0] = items[0];
  person0->items[1] = items[1];
  person0->items[2] = items[2];

  Person *person1 = new_person(2);
  person1->label = 1;
  person1->items[0] = items[3];
  person1->items[1] = items[4];


  // create a gallery
  Gallery *gallery = new_gallery(2);
  gallery->persons[0] = person0;
  gallery->persons[1] = person1;

  // create a database 
  // in fact, its the content length of GItem
  int len = sizeof(int)*2 + sizeof(double)*4800;
  struct db* gdb = createdb(123, "gallery.db", len);
  if (!gdb)
  {
    fprintf(stderr, "can not create databse!\n");
    return ;
  }

  create_gallerydb(gdb, gallery);
  writedb(gdb);
  free_gallery(gallery);
}

void test_load_gallery()
{
  struct db* gdb = fast_opendb("gallery.db");
  if (!gdb)
  {
    fprintf(stderr, "can not open gallery db!\n");
    return ;
  }

  Gallery *gallery = open_gallery(gdb);
  if (!gallery)
  {
    fprintf(stderr, "can not construct gallery from your DB!\n");
    fast_closedb(gdb);	
    return ;
  }
	
  printf("%d\n%d\n", gallery->n, gallery->persons[0]->n);

  for (int i = 0; i < 80; ++i)
  {
    printf("%lf ", gallery->persons[1]->items[1]->feature[i]);
  }

  printf("\n");

  close_gallery(gallery);
	
  fast_closedb(gdb);

}
int main(int argc, char *argv[])
{
  long t = GetTickCount();
  //test_pack();

  //test_gallery();
  
  test_load_gallery();
  
  //test_unpack();
  //test_fast_unpack();
  t = GetTickCount() - t;

  printf("%f", t/1000.);
  getchar();

  return 0;
}
