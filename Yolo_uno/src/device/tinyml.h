#ifndef _TINY_ML_
#define _TINY_ML_

#include <Arduino.h>

#include "globals.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

extern float ml_inference_result; // Biến chia sẻ kết quả

void setupTinyML();
void tiny_ml_task(void *pvParameters);
void tinyML();
#endif