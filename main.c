#include <memory.h>
#include <stdio.h>
#include <stdlib.h>


#define bool int

#define true 1
#define false 0

char buffer[1024 * 1024];
size_t remainbegin = 0;

void *mem_sbrk(size_t size) {

  void *p = &buffer[remainbegin];
  remainbegin += size;
  return p;
}

struct memblock {
  int used; // 0: unused, 1: used
  int size;
  struct memblock *next;
};

struct memblock *freelist = NULL;

bool mm_init() { return true; }

void removeblock(struct memblock *item) {
  if (freelist == item) {
    freelist = NULL;
    return;
  }
  struct memblock *block = freelist;
  for (; block != NULL; block = block->next) {
    if (block->next == item) {
      block->next = item->next;
      return;
    }
  }
  printf("remove fail!");
}

void insertblock(struct memblock *item) {
  struct memblock *block;
  /* front block merge */
  for (block = freelist; block != NULL; block = block->next) {
    if ((void *)(block + 1) + block->size == item) {
      removeblock(block);
      block->size += item->size + sizeof(struct memblock);
      item = block;
    }
  }
  /* back block merge */
  for (block = freelist; block != NULL; block = block->next) {
    if ((void *)(item + 1) + item->size == block) {
      removeblock(block);
      item->size += block->size + sizeof(struct memblock);
    }
  }
  if (freelist == NULL) {
    freelist = item;
    return;
  }
  /* sort insert block */
  struct memblock *prev = NULL;
  for (block = freelist; block != NULL; block = block->next) {
    if (item < block) {
      item->next = block;
      if (prev == NULL) {
        freelist = item;
      } else {
        prev->next = item;
      }
      return;
    }
    prev = block;
  }
  item->next = prev->next;
  prev->next = item;
}

void *splitblock(struct memblock *item, size_t size) {
  if (item->size <= size + sizeof(struct memblock)) {
    struct memblock *next = (void *)item + size + sizeof(struct memblock);
    next->used = 0;
    next->next = NULL;
    next->size = item->size - size - sizeof(struct memblock);
    insertblock(next);
    item->size = size;
  }
  item->used = 1;
  item->next = NULL;
  return (void *)(item + 1);
}

void *malloc(size_t size) {
  struct memblock *item = freelist;
  for (; item != NULL; item = item->next) {
    if (item->size >= size) {
      removeblock(item);
      return splitblock(item, size);
    }
  }
  item = (struct memblock *)mem_sbrk(size + sizeof(struct memblock));
  if (item == NULL) {
    return NULL;
  }
  item->used = 1;
  item->size = size;
  item->next = NULL;
  return (void *)(item + 1);
}

void free(void *ptr) {
  if (NULL == ptr) {
    return;
  }
  struct memblock *item = (struct memblock *)(ptr - sizeof(struct memblock));
  item->used = 0;
  item->next = NULL;
  insertblock(item);
}

int main() {
  void *p = malloc(1024);
  void *p1 = malloc(2048);

  free(p);
  free(p1);

  p = malloc(256);
  p1 = malloc(128);

  free(p);
  free(p1);

  return 0;
}