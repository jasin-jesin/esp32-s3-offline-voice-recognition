# Esp32-s3-offline-voice-recognition
##Project Overview

esp32-s3-offline-voice-recognition enables the ESP32-S3 microcontroller to recognize voice commands completely offline—no cloud, no latency, and your data stays private.

Powered by Espressif’s ESP‑SR library (featuring the MultiNet model), it supports up to 300 Chinese or English voice commands that can be customized on the fly 
Waveshare
+12
GitHub
+12
GitHub
+12
.

Includes an Audio Front-End (AFE) that optimizes input using noise suppression, voice activity detection, and even echo cancellation 
GitHub
.

Offers both single‑recognition mode (detects a command and stops) and continuous‑recognition mode, running until the device times out 
Home Assistant Community
+11
Espressif Docs
+11
Espressif Docs
+11
.

##How It Works

Hardware: Uses the dual-core ESP32‑S3—a microcontroller with AI‑optimized instructions, built‑in Wi‑Fi/Bluetooth, and support for external PSRAM/flash memory 
Wikipedia
+5
Wikipedia
+5
GitHub
+5
.

Voice Input & Processing: Audio is captured via a microphone, pre‑processed by AFE (noise suppression, VAD, etc.), then passed to the MultiNet model.

Command Recognition: The model matches audio input to a spoken command (Chinese or English), with support for up to 300 custom commands dynamically editable 
dfrobot.com
+4
GitHub
+4
Espressif Docs
+4
.

Action Trigger: Recognized commands can trigger hardware or software actions—like toggling LEDs, relaying signals, or interfacing with other peripherals.

##Why Use This Project?

Privacy‑First: Functions completely offline, device does not send your voice data to external servers.

Fast & Efficient: Optimized AI model and AFE deliver low-latency command recognition.

Highly Customizable: Add, remove, or modify commands (both English and Chinese) without retraining the model 
dfrobot.com
+1
.

Edge-Ready: Perfect for smart-home controllers, voice-activated gadgets, or IoT devices where connectivity or privacy is a concern.
##How To Install and Program
## ESP IDF
Make sure you installed the esp idf version 4.4.6. To install type this in your cmd.
```bash
git clone -b v4.4.6 --recursive https://github.com/espressif/esp-idf.git
```
```bash
cd esp-idf-v4.4.6
install.bat
```
Activate the python environment.
```bash
export.bat
```
## ESP SKAINET
Next step is to install esp skainet.
```bash
git clone --branch ESP32-S3-Devkit-C --recursive https://github.com/0015/esp-skainet.git
```
Navigate to esp-skainet.
```bash
cd esp-skainet
```
Next you need to set the I2s Microphone pins.
```bash
cd esp-skainet/components/hardware_driver/boards/include
```
Find the part under @brief ESP32-S3-DEVKIT-C I2S GPIO definition
and substitute with
```bash
#define FUNC_I2S_EN         (1)
#define GPIO_I2S_LRCK       (GPIO_NUM_11)
#define GPIO_I2S_MCLK       (GPIO_NUM_NC)
#define GPIO_I2S_SCLK       (GPIO_NUM_12)
#define GPIO_I2S_SDIN       (GPIO_NUM_10)
#define GPIO_I2S_DOUT       (GPIO_NUM_NC)
```
Next navigate to one example folder 'esp-skainet/examples/en_speech_commands_recognition'.
```bash
cd ../../../../examples/en_speech_commands_recognition/
```
Set esp32 s3 as the board.
```bash
idf.py set-target esp32s3
```
Configure the menu
make sure use set the Audio HAL to ESP32-S3-DEVKIT-C .
```bash
idf.py menuconfig
```
Substitute main.c with [main.c](/main.c)

Now run the build by executing.
```bash
idf.py build
```
Next we flash the code to esp32 s3 and to turn on the monitor.
```bash
idf.py flash monitor
```
Now U can start testing it the default wake command is Hi ESP.

## Custom Commands and GPIO Controls
Open google collab(becuase:-In windows most of the time it's not working)
install gp2-en
```bash
pip install g2p-en
```
install dependencies
```bash
!pip install nltk
import nltk
nltk.download('cmudict')
nltk.download('averaged_perceptron_tagger_eng')
```
Download [multinet_g2p.py](/multinet_g2p.py) and uplad it onto collab files -->content

Substitute turn off led with your command
```bash
!python multinet_g2p.py -t "turn off led"
```
Come back to cmd and run menuconfig again
```bash
idf.py menuconfig
```
Select model ESP Speech Recognition -->  English Speech Commands Model --> english recognition (mn5q8_en)

ctrl+s to save

then move one step backward then --> Add English speech commands

Empty all the id's and substitute with the output you got after running the collab

ctrl+s to save and Q to quit

```bash
idf.py fullclean
```
```bash
idf.py build
```
```bash
idf.py flash monitor
```
Now U can start testing it the default wake command is Hi ESP.
