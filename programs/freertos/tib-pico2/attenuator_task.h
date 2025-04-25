//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef ATTENUATOR_TASK_H
#define ATTENUATOR_TASK_H

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

    void attenuator_task(void *param);

#ifdef __cplusplus
}
#endif


#endif //ATTENUATOR_TASK_H
