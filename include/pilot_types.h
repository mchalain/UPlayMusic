#ifndef __PILOT_TYPES_H__
#define __PILOT_TYPES_H__

#include <stdint.h>
typedef int32_t pilot_coord_t;
typedef uint32_t pilot_length_t;
typedef uint32_t pilot_bitsfield_t;
typedef uint32_t pilot_color_t;
#define colorset	memset32
typedef int16_t pilot_key_t;
typedef char pilot_bool_t;
#ifndef PTHREAD
typedef uint32_t pilot_mutex_t;
#define _pilot_mutex(mutex)	pilot_mutex_t mutex:1
#define mutex_init(mutex, ...) do {mutex = 0; }while(0)
#define mutex_lock(mutex)	!(mutex = (!(mutex))? 1 : 0)
#define mutex_unlock(mutex) do { mutex = 0; }while(0)
#define mutex_destroy(mutex) do {} while(0)

typedef uint32_t pilot_cond_t;
#define _pilot_cond(cond)	pilot_cond_t cond:1
#define cond_init(cond, ...) do { cond = 0; }while(0)
#define cond_wait(cond, mutex)	(cond = ((cond)? 0 : -1))
#define cond_signal(cond) do { cond = 1; }while(0)
#define cond_destroy(cond) do {} while(0)

#else
#include <pthread.h>
typedef pthread_mutex_t pilot_mutex_t;
#define _pilot_mutex(mutex)	pthread_mutex_t mutex
#define mutex_init(mutex, attr) pthread_mutex_init(&mutex, attr);
#define mutex_lock(mutex)	pthread_mutex_lock(&mutex)
#define mutex_unlock(mutex) pthread_mutex_unlock(&mutex)
#define mutex_destroy(mutex) pthread_mutex_destroy(&mutex)

typedef pthread_cond_t pilot_cond_t;
#define _pilot_cond(cond)	pthread_cond_t cond
#define cond_init(cond, attr) pthread_cond_init(&cond, attr);
#define cond_wait(cond, mutex)	pthread_cond_wait(&cond, &mutex)
#define cond_signal(cond) pthread_cond_signal(&cond)
#define cond_destroy(cond) pthread_cond_destroy(&cond)

#endif

struct pilot_rect
{
	pilot_coord_t x;
	pilot_coord_t y;
	pilot_length_t w;
	pilot_length_t h;
};
typedef struct pilot_rect pilot_rect_t;
#define pilot_rect_copy(dest, src) do { memcpy(dest, src, sizeof(struct pilot_rect)); } while(0)

typedef enum
{
	PILOT_DISPLAY_ARGB8888,
	PILOT_DISPLAY_XRGB8888
} pilot_pixel_format_t;
#endif
