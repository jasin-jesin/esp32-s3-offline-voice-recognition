#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_iface.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "esp_board_init.h"
#include "speech_commands_action.h"
#include "model_path.h"
#include "esp_process_sdkconfig.h"


#define OUTPUT_GPIO GPIO_NUM_7
#define OUTPUT_GPIO2 GPIO_NUM_6
#define OUTPUT_GPIO3 GPIO_NUM_5

int detect_flag = 0;
static esp_afe_sr_iface_t *afe_handle = NULL;
static volatile int task_flag = 0;
srmodel_list_t *models = NULL;
static int play_voice = -2;

void play_music(void *arg)
{
    while (task_flag) {
        switch (play_voice) {
        case -2:
            vTaskDelay(10);
            break;
        case -1:
            wake_up_action();
            play_voice = -2;
            break;
        default:
            speech_commands_action(play_voice);
            play_voice = -2;
            break;
        }
    }
    vTaskDelete(NULL);
}

void feed_Task(void *arg)
{
    esp_afe_sr_data_t *afe_data = arg;
    int audio_chunksize = afe_handle->get_feed_chunksize(afe_data);
    int nch = afe_handle->get_channel_num(afe_data);
    int feed_channel = esp_get_feed_channel();
    assert(nch <= feed_channel);
    int16_t *i2s_buff = malloc(audio_chunksize * sizeof(int16_t) * feed_channel);
    assert(i2s_buff);

    while (task_flag) {
        esp_get_feed_data(false, i2s_buff, audio_chunksize * sizeof(int16_t) * feed_channel);
        afe_handle->feed(afe_data, i2s_buff);
    }

    if (i2s_buff) {
        free(i2s_buff);
    }
    vTaskDelete(NULL);
}

void detect_Task(void *arg)
{
    esp_afe_sr_data_t *afe_data = arg;
    int afe_chunksize = afe_handle->get_fetch_chunksize(afe_data);
    char *mn_name = esp_srmodel_filter(models, ESP_MN_PREFIX, ESP_MN_ENGLISH);
    printf("multinet:%s\n", mn_name);
    esp_mn_iface_t *multinet = esp_mn_handle_from_name(mn_name);
    model_iface_data_t *model_data = multinet->create(mn_name, 6000);
    int mu_chunksize = multinet->get_samp_chunksize(model_data);
    esp_mn_commands_update_from_sdkconfig(multinet, model_data);
    assert(mu_chunksize == afe_chunksize);
    multinet->print_active_speech_commands(model_data);

    printf("------------detect start------------\n");

    while (task_flag) {
        afe_fetch_result_t* res = afe_handle->fetch(afe_data); 
        if (!res || res->ret_value == ESP_FAIL) {
            printf("fetch error!\n");
            break;
        }

        if (res->wakeup_state == WAKENET_DETECTED) {
            printf("WAKEWORD DETECTED\n");
            multinet->clean(model_data);
        } else if (res->wakeup_state == WAKENET_CHANNEL_VERIFIED) {
            play_voice = -1;
            detect_flag = 1;
            printf("AFE_FETCH_CHANNEL_VERIFIED, channel index: %d\n", res->trigger_channel_id);
        }

        if (detect_flag == 1) {
            esp_mn_state_t mn_state = multinet->detect(model_data, res->data);
            if (mn_state == ESP_MN_STATE_DETECTING) continue;

            if (mn_state == ESP_MN_STATE_DETECTED) {
                esp_mn_results_t *mn_result = multinet->get_results(model_data);
                for (int i = 0; i < mn_result->num; i++) {
                    printf("TOP %d, command_id: %d, phrase_id: %d, string: %s, prob: %f\n", 
                        i+1, mn_result->command_id[i], mn_result->phrase_id[i], mn_result->string, mn_result->prob[i]);
                }

                // Perform GPIO action
                switch(mn_result->phrase_id[0]){
                case 0:
                    gpio_set_level(OUTPUT_GPIO, 1);
                    printf("GPIO 7 set HIGH\n");
                    break;
                case 1:
                    gpio_set_level(OUTPUT_GPIO, 0); 
                    printf("GPIO 7 set LOW\n");
                    break;
                case 2:
                    gpio_set_level(OUTPUT_GPIO2, 0); 
                    printf("GPIO 6 set LOW\n");
                    break;
                case 3:
                    gpio_set_level(OUTPUT_GPIO2, 1); 
                    printf("GPIO 6 set HIGH\n");
                    break;
                case 4:
                    gpio_set_level(OUTPUT_GPIO3, 1); 
                    printf("GPIO 5 set HIGH\n");
                    break;
                case 5:
                    gpio_set_level(OUTPUT_GPIO3, 0); 
                    printf("GPIO 5 set LOW\n");
                    break;
                }

                detect_flag = 0;
                printf("-----------listening-----------\n");
            }

            if (mn_state == ESP_MN_STATE_TIMEOUT) {
                esp_mn_results_t *mn_result = multinet->get_results(model_data);
                printf("timeout, string:%s\n", mn_result->string);
                afe_handle->enable_wakenet(afe_data);
                detect_flag = 0;
                printf("\n-----------awaits to be waken up-----------\n");
            }
        }
    }
    if (model_data) {
        multinet->destroy(model_data);
    }
    printf("detect exit\n");
    vTaskDelete(NULL);
}

void app_main()
{
    models = esp_srmodel_init("model");
    ESP_ERROR_CHECK(esp_board_init(AUDIO_HAL_16K_SAMPLES, 1, 16));

#if CONFIG_IDF_TARGET_ESP32
    printf("This demo only supports ESP32S3\n");
    return;
#else 
    afe_handle = (esp_afe_sr_iface_t *)&ESP_AFE_SR_HANDLE;
#endif

    // Setup GPIO 7 as output
    gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << OUTPUT_GPIO) | (1ULL << OUTPUT_GPIO2) | (1ULL << OUTPUT_GPIO3),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};
    gpio_config(&io_conf);
    gpio_config(&io_conf);
    gpio_set_level(OUTPUT_GPIO, 0);  // Start LOW

    // Start AFE
    afe_config_t afe_config = AFE_CONFIG_DEFAULT();
    afe_config.wakenet_model_name = esp_srmodel_filter(models, ESP_WN_PREFIX, NULL);
#if defined CONFIG_ESP32_S3_BOX_BOARD || defined CONFIG_ESP32_S3_EYE_BOARD || CONFIG_ESP32_S3_DEVKIT_C
    afe_config.aec_init = false;
    #if defined CONFIG_ESP32_S3_EYE_BOARD || CONFIG_ESP32_S3_DEVKIT_C
        afe_config.pcm_config.total_ch_num = 2;
        afe_config.pcm_config.mic_num = 1;
        afe_config.pcm_config.ref_num = 1;
    #endif
#endif

    esp_afe_sr_data_t *afe_data = afe_handle->create_from_config(&afe_config);

    task_flag = 1;
    xTaskCreatePinnedToCore(&detect_Task, "detect", 8 * 1024, (void*)afe_data, 5, NULL, 1);
    xTaskCreatePinnedToCore(&feed_Task, "feed", 8 * 1024, (void*)afe_data, 5, NULL, 0);
    xTaskCreatePinnedToCore(&play_music, "play", 4 * 1024, NULL, 5, NULL, 1);
}
