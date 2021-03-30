#ifndef DEV_LOGGER
#define DEV_LOGGER

#include <stdio.h>

/**@brief Log priority levels */

#define LOG_LEVEL_DEBUG      0
#define LOG_LEVEL_INFO       1
#define LOG_LEVEL_WARNING    2
#define LOG_LEVEL_ERROR      3
#define LOG_LEVEL_ASSERT     4
#define LOG_LEVEL_TRACE      5
#define LOG_LEVEL_NONE       6

/**@brief Current log level */
#define LOG_LEVEL LOG_LEVEL_DEBUG

#ifndef LOG_LEVEL
    #error "LOG_LEVEL must be defined"
#endif

/**@brief Debug logger */

#if (LOG_LEVEL_DEBUG >= LOG_LEVEL)
    #define LOG_DEBUG        printf("[ST25]:[%s][DEBUG] ", __FUNCTION__);printf
#else
    #define LOG_DEBUG(...)   ((void) 0)
#endif

/**@brief Info logger */

#if (LOG_LEVEL_INFO >= LOG_LEVEL)
    #define LOG_INFO         printf("[ST25]:[%s][INFO] ", __FUNCTION__);printf
#else
    #define LOG_INFO(...)    ((void) 0)
#endif

/**@brief Warning logger */

#if (LOG_LEVEL_WARNING >= LOG_LEVEL)
    #define LOG_WARNING      printf("[ST25]:[%s][WARNING] ", __FUNCTION__);printf
#else
    #define LOG_WARNING(...) ((void) 0)
#endif

/**@brief Error logger */

#if (LOG_LEVEL_ERROR >= LOG_LEVEL)
    #define LOG_ERROR        printf("[ST25]:[%s][ERROR] ", __FUNCTION__);printf
#else
    #define LOG_ERROR(...)   ((void) 0)
#endif

/**@brief Assert logger */

#if (LOG_LEVEL_ASSERT >= LOG_LEVEL)
    #define LOG_ASSERT       printf("[ST25]:[%s][ASSERT] ", __FUNCTION__);printf
#else
    #define LOG_ASSERT(...)  ((void) 0)
#endif

/**@brief Assert logger */

#if (LOG_LEVEL_TRACE >= LOG_LEVEL)
    #define LOG_TRACE       printf("[ST25]:[%s][TRACE] ", __FUNCTION__);printf
#else
    #define LOG_TRACE(...)  ((void) 0)
#endif

#define LOG_TEST       printf("[ST25]:[%s][TEST] ", __FUNCTION__);printf

static inline void LOG_DUMP(const uint8_t *packet, size_t length)
{
  char output[sizeof("xxxxyyyy xxxxyyyy")];
  int n = 0, k = 0;
	LOG_DEBUG("DUMP data %d\n", length);
  uint8_t byte;
  while (length--) {
    if (n % 16 == 0) {
      printf(" %08X ", n);
    }

    byte = *packet++;

    printf("%02X ", byte);

    if (byte < 0x20 || byte > 0x7f) {
      output[k++] = '.';
    } else {
      output[k++] = byte;
    }

    n++;
    if (n % 8 == 0) {
      if (n % 16 == 0) {
        output[k] = '\0';
        printf(" [%s]\n", output);
        k = 0;
      } else {
        printf(" ");
      }
    }
  }

  if (n % 16) {
    int i;

    output[k] = '\0';

    for (i = 0; i < (16 - (n % 16)); i++) {
      printf("   ");
    }

    if ((n % 16) < 8) {
      printf(" "); /* one extra delimiter after 8 chars */
    }

    printf(" [%s]\n", output);
  }
}

#endif 
