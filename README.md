# esp32-s3-offline-voice-recognition

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
Download [multinet_g2p.py](./src/multinet_g2p.py) and uplad it onto collab files -->content