//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef ADC_TASK_H
#define ADC_TASK_H


#pragma once

// adc_task.hpp
#pragma once

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

    void adc_task(void *param);

#ifdef __cplusplus
}
#endif


#endif //ADC_TASK_H

