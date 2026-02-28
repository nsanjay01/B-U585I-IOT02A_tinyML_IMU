# B-U585I-IOT02A_tinyML_IMU
Using B-U585I-IOT02A and on board IMU sensor, building TinyML application

## Edge Anomaly Detection: TinyML LSTM Autoencoder on STM32
This repository contains a full-stack TinyML implementation of an Anomaly Detection System running on an ARM Cortex-M33 microcontroller. The system uses an LSTM Autoencoder to learn the "temporal signature" of normal human activity and flags deviations (anomalies) in real-time.

### Key Features

* **Temporal Intelligence**: Unlike simple thresholding, this uses an LSTM to understand the sequence and rhythm of motion.
* **Ultra-Lightweight**: Compressed using INT8 Post-Training Quantization to fit into a tiny memory footprint.
* **Hardware-Accelerated**: Runs on the STM32U585 IoT Discovery Kit, utilizing its high-performance DSP capabilities.
* **Real-Time Processing**: Inference executes in <10ms, processing data from the onboard ISM330DHCX 6-axis IMU.

### The Architecture: Why an Autoencoder?

Classification models tell you what an activity is. An Autoencoder tells you if an activity is wrong.

1. **Compression**: The model takes 25 samples of motion and compresses them into a "bottleneck" layer.
2. **Reconstruction**: It then attempts to rebuild the original 25 samples.
3. **The Error (MSE)**: Since the model was trained only on "normal" activities (walking, sitting, standing), it is an expert at reconstructing those patterns. If you perform an "abnormal" motion (shaking, falling, crashing), the model fails to reconstruct it accurately.
4. **Detection**: A high Mean Squared Error (MSE) triggers the onboard Red LED.


### Hardware & Software Stack

* **MCU**: STM32U585 (Cortex-M33 @ 160MHz)
* **Sensors**: ISM330DHCX (Accelerometer & Gyroscope)
* **Dataset**: UCI Human Activity Recognition (HAR)
* **Inference Engine**: TensorFlow Lite for Microcontrollers (TFLM)
* **Build System**: CMake + ARM GCC

### Performance Metrics

After unrolling the LSTM cells and applying INT8 quantization, the model achieves impressive efficiency:
Metric                     Value
> Inference Time           ~7.2 ms,
> RAM (Tensor Arena)       100 KB (Optimized),
> Model Size               ~42 KB (Flash),
> Sensor Frequency         52 Hz
***

### Installation & Usage

**1. Model Training (Google Colab or on local)**
The training script includes resampling to 25Hz and INT8 quantization logic.
* Run the script to generate `model_data.h`


**2. Embedded Deployment (CMake)**
Ensure you have the ARM GNU Toolchain and CMake installed.
```
mkdir build
cd build
cmake ..
make -j4
```

**3. Monitoring**
Connect to the board via Serial (115200 baud). You will see real-time MSE scores:
```
Normal Operation
SCORE: 0.035 | LIMIT: 0.054 | Counter: 0
Normal Operation
SCORE: 0.167 | LIMIT: 0.054 | Counter: 0
Normal Operation
SCORE: 0.126 | LIMIT: 0.054 | Counter: 1
Normal Operation
SCORE: 0.294 | LIMIT: 0.054 | Counter: 2
Anomaly Detected! Score: 
SCORE: 0.249 | LIMIT: 0.054 | Counter: 3
Anomaly Detected! Score: 
SCORE: 0.240 | LIMIT: 0.054 | Counter: 4
Anomaly Detected! Score: 
SCORE: 0.138 | LIMIT: 0.054 | Counter: 5
Anomaly Detected! Score: 
SCORE: 0.218 | LIMIT: 0.054 | Counter: 6
```

### Acknowledgements
* [UCI Machine Learning Repository: HAR Dataset ](https://archive.ics.uci.edu/dataset/240/human+activity+recognition+using+smartphones)
* [TensorFlow Lite Micro Documentation](https://www.tensorflow.org/lite/microcontrollers)
* [ISM330DHCX Datasheet (STMicroelectronics)](https://www.st.com/resource/en/datasheet/ism330dhcx.pdf)
