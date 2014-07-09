#ifndef _pilot_list
#define _pilot_list(type, name) struct _pilot_list_##type {\
			struct type *item; \
			struct _pilot_list_##type *next; \
			struct _pilot_list_##type *it; \
		} name
#define pilot_list_append(list, entry) do { typeof (list) *it = &list; \
			while (it->next) it = it->next; \
			it->next = malloc(sizeof(typeof (list))); \
			it = it->next; \
			memset(it, 0, sizeof(typeof (list))); \
			it->item = entry; \
		} while(0)
#define pilot_list_insert(list, entry, index) do { typeof (list) *it = &list; \
			int i; \
			for (i = 0; i < index && it->next; i++) it = it->next; \
			typeof (list) *new = malloc(sizeof(typeof (list))); \
			memset(new, 0, sizeof(typeof (list))); \
			new->item = entry; \
			new->next = it->next; \
			it->next = new; \
		} while(0)
#define pilot_list_remove(list, entry) do { typeof (list) *it = &list; \
			while (it->next && it->next->item != entry) it = it->next; \
			if (it->next) { \
				it->it = it->next; \
				it->next = it->next->next; \
				free(it->it); \
				it->it = NULL; \
			} \
		} while(0)
#define pilot_list_first(list) (list.next)?list.next->item:NULL; list.it = list.next
#define pilot_list_next(list) (list.it->next)?list.it->next->item:NULL;list.it = list.it->next;
#define pilot_list_foreach(list, func, data, ...) do { list.it = list.next; \
			while (list.it) { \
				typeof (list) *it = list.it; \
				list.it = list.it->next; \
				if (func(data, it->item, ##__VA_ARGS__) < 0) break; \
			} \
		} while(0)
#define pilot_list_destroy(list) do { list.it = list.next; \
			while (list.it) { \
				typeof (list) *tmp = list.it; \
				list.it = list.it->next; \
				free(tmp); \
			} \
		} while(0)
#endif
