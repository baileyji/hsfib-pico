//
// Created by Jeb Bailey on 4/23/25.
//

#ifndef PUB_TASK_H
#define PUB_TASK_H

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

    void pub_task(void *param);

#ifdef __cplusplus
}
#endif

#endif // PUB_TASK_H
