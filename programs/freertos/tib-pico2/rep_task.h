//
// Created by Jeb Bailey on 4/23/25.
//

#ifndef REP_TASK_H
#define REP_TASK_H

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

    void rep_task(void *param);

#ifdef __cplusplus
}
#endif

#endif //REP_TASK_H
