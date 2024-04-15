#include <windows.h>
typedef struct {
  HANDLE h;
  char *name;
  int piece_start;
  int piece_end;
  int index;
  BOOL finished;
  int last_downloaded_piece;
} FileInfo;
typedef struct {
  FileInfo *items;
  int count;
  int size;
} HandlesMap;

FileInfo *map_get(HandlesMap *map, HANDLE h) {
  for (int i = 0; i < map->count; i++) {
    if (map->items[i].h == h) {
      return &map->items[i];
    }
  }
  return NULL;
}

void map_add(HandlesMap *map, FileInfo info) {
  if (map->count == map->size) {
    FileInfo *items = malloc(sizeof(FileInfo) * map->size * 2);
    for (int i = 0; i < map->count; i++) {
        items[i] = map->items[i];
    }
    free(map->items);
    map->items = items;
    map->size *= 2;
  }
  map->items[map->count++] = info;
}
HandlesMap *map_new() {
    HandlesMap *map = malloc(sizeof(HandlesMap));
    map->size = 8;
    map->count = 0;
    map->items = malloc(sizeof(FileInfo) * map->size);
    return map;
}
