#pragma once

typedef struct ListItem {
  struct ListItem* prev;
  struct ListItem* next;
} ListItem;

typedef struct ListHead {
  ListItem* first;
  ListItem* last;
  int size;
} ListHead;

void List_init(ListHead* head);
ListItem* List_find(ListHead* head, ListItem* item);
ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
ListItem* List_detach(ListHead* head, ListItem* item);
ListItem* List_popFront(ListHead* head);
ListItem* List_popBack(ListHead* head);
ListItem* List_pushFront(ListHead* head, ListItem* item);
ListItem* List_pushBack(ListHead* head, ListItem* item);
