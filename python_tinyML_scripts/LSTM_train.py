#This is originally a google colab code, but you can also use it to train locally, 
# make sure dependencies are installed.

import os
import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras import layers, models
from scipy import signal
import matplotlib.pyplot as plt

# 1. Download and Extract Data
!wget -N "https://archive.ics.uci.edu/ml/machine-learning-databases/00240/UCI HAR Dataset.zip"
!unzip -q -o "UCI HAR Dataset.zip"

def load_signals(subset):
    signals_list = [
        'total_acc_x', 'total_acc_y', 'total_acc_z',
        'body_gyro_x', 'body_gyro_y', 'body_gyro_z'
    ]
    path = f"UCI HAR Dataset/{subset}/Inertial Signals/"
    path_test = f"UCI HAR Dataset/test/Inertial Signals/"
    data = []
    test_data = []

    for sig in signals_list:
        filename = f"{path}{sig}_{subset}.txt"
        data.append(pd.read_csv(filename, header=None, delim_whitespace=True).values)

    for sig in signals_list:
      filename = f"{path_test}{sig}_test.txt"
      test_data.append(pd.read_csv(filename, header=None, delim_whitespace=True).values)

    return np.transpose(np.array(data), (1, 2, 0)), np.transpose(np.array(test_data),(1,2,0))

# 2. Load, Resample, and Scale
x_train_uci, test_train_uci = load_signals('train')

# --- CHANGED FROM 50 to 25 ---
WINDOW_SIZE = 25 
x_train_uci = signal.resample(x_train_uci, WINDOW_SIZE, axis=1)
test_train_uci = signal.resample(test_train_uci, WINDOW_SIZE, axis=1)

x_train_uci[:,:,3:] = (x_train_uci[:,:,3:] * 57.2958) / 250.0
test_train_uci[:,:,3:] = (test_train_uci[:,:,3:] * 57.2958) / 250.0

# --- TRAINING ---
print("\n--- Training Model ---")
# Using the standard model for training
model = models.Sequential([
    layers.Input(shape=(WINDOW_SIZE, 6)),
    layers.LSTM(32, return_sequences=False),
    layers.Reshape((1, 32)),
    layers.LSTM(32, return_sequences=True),
    layers.Dense(WINDOW_SIZE * 6),
    layers.Reshape((WINDOW_SIZE, 6))
])
model.compile(optimizer='adam', loss='mse')

history = model.fit(
    x_train_uci, x_train_uci,
    epochs=30,
    batch_size=64,
    validation_split=0.1
)

# 4. Evaluate and Find Threshold
print("\n--- Calculating Threshold ---")
predictions = model.predict(test_train_uci)
mse_per_window = np.mean(np.power(test_train_uci - predictions, 2), axis=(1, 2))

mean_mse = np.mean(mse_per_window)
std_mse = np.std(mse_per_window)
threshold = mean_mse + (std_mse * 3)

print(f"Mean MSE: {mean_mse:.6f}")
print(f"Standard Deviation: {std_mse:.6f}")
print(f"Recommended Threshold: {threshold:.6f}")

# ---------------------------------------------------------
# Static + Unrolled 
# ---------------------------------------------------------
print("\n--- Creating Static Model for Conversion ---")

static_model = models.Sequential([
    layers.Input(batch_shape=(1, WINDOW_SIZE, 6)),
    layers.LSTM(32, return_sequences=False, unroll=True),
    layers.Reshape((1, 32)),
    layers.LSTM(32, return_sequences=True, unroll=True),
    layers.Dense(WINDOW_SIZE * 6),
    layers.Reshape((WINDOW_SIZE, 6))
])

static_model.set_weights(model.get_weights())

# 5. TFLite INT8 Conversion
print("\n--- Converting to TFLite (INT8) ---")
converter = tf.lite.TFLiteConverter.from_keras_model(static_model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]

def representative_dataset():
    for i in range(100):
        data = np.expand_dims(x_train_uci[i], axis=0).astype(np.float32)
        yield [data]

converter.representative_dataset = representative_dataset
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

converter._experimental_disable_per_channel_quantization_for_dense_layers = True
converter._experimental_lower_tensor_list_ops = True
converter._experimental_default_to_single_batch_in_tensor_list_ops = True 

try:
    tflite_model = converter.convert()
    with open('anomaly_model.tflite', 'wb') as f:
        f.write(tflite_model)
    print("SUCCESS: anomaly_model.tflite created!")

    def hex_to_c_array(hex_data, var_name):
        c_str = f"alignas(16) const unsigned char {var_name}[] = {{\n  "
        for i, val in enumerate(hex_data):
            c_str += f"0x{val:02x}, "
            if (i + 1) % 12 == 0:
                c_str += "\n  "
        c_str = c_str[:-2] + "\n};\n"
        c_str += f"constexpr unsigned int {var_name}_len = {len(hex_data)};\n"
        return c_str

    with open('model_data.h', 'w') as f:
        f.write(hex_to_c_array(tflite_model, "anomaly_model_tflite"))
    print("SUCCESS: model_data.h created!")
except Exception as e:
    print(f"\n>>> CONVERSION FAILED! Exact error: {e} <<<\n")