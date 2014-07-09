#ifndef __PILOT_SIGNAL_H__
#define __PILOT_SIGNAL_H__

#define _pilot_signal(obj, signal,...) \
	struct { \
		struct obj##_##signal##_slot{ \
			void *dest; \
			struct obj##_##signal##_slot *next; \
			struct obj##_##signal##_slot *it; \
			int(*action)(void *, ##__VA_ARGS__); \
		} slots; \
	} signal

#define pilot_connect(src, signal, dst, slot) \
	do { \
		typeof(src->signal.slots) *slot_it = &src->signal.slots; \
		while (slot_it->next) { \
			if ((void *)slot_it->next->dest == (void *)dst && (void *)slot_it->next->action == (void *)slot) \
				break; \
			slot_it = slot_it->next; \
		} \
		if (slot_it->next == NULL) { \
			slot_it->next = malloc(sizeof(*slot_it)); \
			slot_it->next->next = NULL; \
			slot_it->next->dest = (void *)dst; \
			slot_it->next->action = (void *)slot; \
		} \
	} while(0)

#define pilot_disconnect(src, signal, dst) \
	do { \
		typeof(src->signal.slots) *slot_it = &src->signal.slots; \
		while (slot_it->next) { \
			if ((void *)slot_it->next->dest == (void *)dst) { \
				typeof(src->signal.slots) *tmp = slot_it->next; \
				slot_it->next = tmp->next; \
				free(tmp); \
			} \
			else \
				slot_it = slot_it->next; \
		} \
	} while(0)

#define pilot_emit(src, signal, ...) \
	do { \
		src->signal.slots.it = src->signal.slots.next; \
		while (src->signal.slots.it) { \
			typeof(src->signal.slots) *slot_it = src->signal.slots.it; \
			src->signal.slots.it = src->signal.slots.it->next; \
			if (slot_it->action(slot_it->dest, ##__VA_ARGS__) < 0) break; \
		} \
	} while(0)

/*
#define pilot_emit(src, signal, ...) \
	do { \
		typeof(src->signal.slots) *slot_it = &src->signal.slots; \
		while (slot_it->next) { \
			slot_it = slot_it->next; \
			if (slot_it->action(slot_it->dest, ##__VA_ARGS__) < 0) break; \
		} \
	} while(0)
*/
#define pilot_emit_to(src, signal, dst, ...) \
	do { \
		typeof(src->signal.slots) *slot_it = &src->signal.slots; \
		while (slot_it->next) { \
			slot_it = slot_it->next; \
			if ((void *)dst == (void *)slot_it->dest) \
				slot_it->action(slot_it->dest, ##__VA_ARGS__); \
		} \
	} while(0)

#define pilot_signal_debug(src, signal) \
	do { \
		typeof(src->signal.slots) *slot_it = &src->signal.slots; \
		while (slot_it->next) { \
			slot_it = slot_it->next; \
			LOG_DEBUG("signal ##signal connected to function %p of %p", slot_it->dest, slot_it->action); \
		} \
	} while(0)

#endif
