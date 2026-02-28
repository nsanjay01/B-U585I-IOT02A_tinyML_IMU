#include "app_main.h"
#include "main.h"
#include <math.h>
#include <string.h>

#include "tensorflow/lite/core/c/common.h"
#include "model.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/recording_micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
// Include our TFLM headers here
constexpr int tensor_arena_size = 100 * 1024;



alignas(16) uint8_t tensor_arena[tensor_arena_size];
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

void AI_Init() {
    tflite::InitializeTarget();

    // 1. Load the model
    model = tflite::GetModel(anomaly_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        MicroPrintf("Model schema mismatch!");
        return;
    }

    // 2. Register only the ops we need (LSTM + Dense + Helpers)
    // The <6> tells the resolver to reserve space for 6 operators
    static tflite::MicroMutableOpResolver<150> resolver;
    resolver.AddWhile();
    resolver.AddStridedSlice();
    resolver.AddUnidirectionalSequenceLSTM();
    resolver.AddFullyConnected(); // For the Dense layer
    resolver.AddReshape();        // Often used by RepeatVector
    resolver.AddQuantize();       // Needed if model is quantized
    resolver.AddDequantize();     // Needed if model is quantized
    resolver.AddMul();            // Helper for LSTM math
    resolver.AddSub();
    resolver.AddLess();
    resolver.AddAdd();
    resolver.AddLogicalAnd();
    resolver.AddGather();
    resolver.AddSplit();
    resolver.AddRelu();
    resolver.AddConcatenation();
    resolver.AddLogistic();
    resolver.AddSlice();
    resolver.AddUnpack();
    resolver.AddTanh();

    
   

    

    // 3. Build the interpreter
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, tensor_arena_size);
    interpreter = &static_interpreter;

    // 4. Allocate tensors
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        MicroPrintf("AllocateTensors failed!");
        return;
    }

    // 5. Map input/output pointers
    input = interpreter->input(0);
    output = interpreter->output(0);
}

float Run_Inference(float* window_data) {
    // Total elements = 25 samples * 6 axes (acc x,y,z + gyro x,y,z)
    const int num_elements = 150;

    // -----------------------------------------------------------------
    // 1. QUANTIZE INPUT (Float -> Int8)
    // -----------------------------------------------------------------
    float input_scale = input->params.scale;
    int input_zero_point = input->params.zero_point;
    
    // Ensure we are using the int8 pointer for the quantized model
    int8_t* input_tensor_data = input->data.int8;

    for (int i = 0; i < num_elements; i++) {
        // Quantization formula: (value / scale) + zero_point
        float val = (window_data[i] / input_scale) + input_zero_point;

        // Clamp to Int8 range [-128, 127]
        if (val > 127.0f) val = 127.0f;
        if (val < -128.0f) val = -128.0f;

        input_tensor_data[i] = (int8_t)(roundf(val));
    }

    // -----------------------------------------------------------------
    // 2. RUN INFERENCE
    // -----------------------------------------------------------------
    if (interpreter->Invoke() != kTfLiteOk) {
        MicroPrintf("Invoke failed!");
        return -1.0f; 
    }

    // -----------------------------------------------------------------
    // 3. DEQUANTIZE OUTPUT & CALCULATE MSE (Int8 -> Float)
    // -----------------------------------------------------------------
    float output_scale = output->params.scale;
    int output_zero_point = output->params.zero_point;
    int8_t* predicted_q = output->data.int8;
    
    float total_mse = 0.0f;
    
    for (int i = 0; i < num_elements; i++) { 
        // Dequantization formula: (quantized_value - zero_point) * scale
        float predicted_f = (predicted_q[i] - output_zero_point) * output_scale;
        
        // Calculate squared error against the original input
        float diff = window_data[i] - predicted_f;
        total_mse += (diff * diff);
    }
    
    // Return average MSE for the 25-sample window
    return total_mse / (float)num_elements;
}


int app_main(void){
   return 0;
}