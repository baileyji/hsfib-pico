//
// Created by Jeb Bailey on 4/23/25.
//

#ifndef SWITCHING_TASK_H
#define SWITCHING_TASK_H


#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

    void switching_task(void *param);

#ifdef __cplusplus
}
#endif


#endif //SWITCHING_TASK_H
