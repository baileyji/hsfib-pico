//
// Created by Jeb Bailey on 4/25/25.
//

#ifndef EXECUTOR_TASK_H
#define EXECUTOR_TASK_H

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

    void executor_task(void *param);

#ifdef __cplusplus
}
#endif



#endif //EXECUTOR_TASK_H
