#ifndef __USER_CONFIG_BASE_H__
#define __USER_CONFIG_BASE_H__

#define USE_OPTIMIZE_PRINTF
#include "user_config.local.h"

#if defined(DEBUG_ON)
#define INFO( format, ... ) os_printf( "%s - INFO - "format, __FILE__, ## __VA_ARGS__ )
#define ERROR( format, ... ) os_printf( "%s - ERROR - "format, __FILE__, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#define ERROR( format, ... )
#endif

#endif

