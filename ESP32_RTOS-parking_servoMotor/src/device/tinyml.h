#ifndef __TINY_ML__
#define __TINY_ML__

#include <Arduino.h>

#include "globals.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

void setupTinyML();
void tiny_ml_task(void *pvParameters);
void tinyML(); 
extern const unsigned char dht_anomaly_model_tflite[];
extern float ml_inference_result; // Biến chia sẻ kết quả

#endif // __TINY_ML__