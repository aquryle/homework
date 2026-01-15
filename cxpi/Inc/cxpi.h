#ifndef __CXPI_H__
#define __CXPI_H__

#include "cxpi_user.h"


#if (CXPI_MODE == CXPI_POLLING)
#include "cxpi_polling.h"
#elif (CXPI_MODE == CXPI_EVENT)
#include "cxpi_event.h"
#endif




#endif
