/***********************************************
 *
 *    KOOL KEG CODE
 *
 *    Written by:
 *    -  Mitchell Morris
 *    -  Jared Nutt
 *    -  Cole Kniffen
 *    -  Dylan Huntoon
 *
 *    API's used:
 *
 *    HX711 ESP-IDF Driver Library: https://github.com/akshayvernekar/HX711_IDF,
 *    MLX90614 ESP-IDF Driver Library: https://github.com/mistak1992/MLX90614_driver_for_esp_idf
 *    ESP-IDF I2C EEPROM Library: https://github.com/zacharyvincze/esp32-i2c-eeprom
 *    ESP-IDF http client Driver Library: https://github.com/espressif/esp-idf/blob/master/examples/protocols/esp_http_client/main/esp_http_client_example.c
 *    ESP-IDF Pulse Count Library: https://docs.espressif.com/projects/espressif-esp-idf/en/latest/api-reference/peripherals/pcnt.html
 *    GPIO Interrupt Service Routine Library: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
 *
 ***********************************************/


// Include List

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "eeprom.h"
#include "MLX90614_API.h"
#include "MLX90614_SMBus_Driver.h"
#include "i2c_lcd.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include <stdbool.h>
#include <unistd.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_pm.h"
#include "HX711.h"



//************ FIX THE BUTTONS FOR GPIO*************//

int pointerval;
const uint8_t eeprom_address = 0x50; // Address of EEPROM
const uint16_t starting_address = 0x0000; // Start writing to byte 0x00, increment starting address by 1 in order to write to the next byte
EventBits_t bits;

//Weight Sensor Initialization

#define AVG_SAMPLES   1 // Used to take give user an average value of weight in order to improve accuracy
#define GPIO_DATA   26
#define GPIO_SCLK   27

//Pulse Counter Initialization

#define EXAMPLE_PCNT_HIGH_LIMIT 100
#define EXAMPLE_PCNT_LOW_LIMIT  -100
#define EXAMPLE_EC11_GPIO_A 4


// MLX90164 Initialization

#define MLX90614_DEFAULT_ADDRESS 0x5A // default chip address(slave address) of MLX90614
#define MLX90614_SDA_GPIO 21 // sda for MLX90614
#define MLX90614_SCL_GPIO 22 // scl for MLX90614


// Global Variables

int selectedvol; // Volume selected by user: initialized to be 8 in main
double num; // Temperature to be displayed after first pour
float volleft; // Volume left in keg to be displayed after first pour

// WIFI initializations

#define EXAMPLE_ESP_WIFI_SSID      "Mitchi iPhone12"
#define EXAMPLE_ESP_WIFI_PASS      "umez6i4dbeiu3"
#define ESP_MAXIMUM_RETRY 2
int sta_retry_num = 0;
EventGroupHandle_t sta_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static const char *TAG = "wifi station";



// I2C master init for OLED Display Only

static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
    	.clk_flags=0,
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_18,
        .scl_io_num = GPIO_NUM_19,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };

    i2c_param_config(I2C_NUM_1, &conf);

    return i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0);
}




void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{

    // patched up event handler ...

    if (event_base == WIFI_EVENT)

    switch (event_id){
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            ESP_LOGI("WIFI_STA", "connecting to AP...");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI("WIFI_STA", "connectes to AP");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            xEventGroupClearBits(sta_wifi_event_group, WIFI_CONNECTED_BIT);
            if (sta_retry_num < ESP_MAXIMUM_RETRY) {
                esp_wifi_connect();
                sta_retry_num++;
                ESP_LOGI("WIFI_STA", "retry to connect to AP");
            } else {
                xEventGroupSetBits(sta_wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGI("WIFI_STA", "could not connect to AP");
            };
            break;
        default:
            ESP_LOGI("WIFI_STA", "unhandled WITI event: ID=1");
        }

        if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI("WIFI_STA", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            sta_retry_num = 0;
            xEventGroupSetBits(sta_wifi_event_group, WIFI_CONNECTED_BIT);
            xEventGroupClearBits(sta_wifi_event_group, WIFI_FAIL_BIT);
        }
}



// LCD Display Initialization
char buffer[16];



// Initialization

//#define GPIO_OUTPUT_IO_0    15 // Left unconnected, remove later
//#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0)

// Trigger for the dispense button change to desired gpio later
#define GPIO_INPUT_IO_0     25
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0



// Send data to the cloud via post/put method
esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}



static void post_rest_function(char *post_data)
{

    esp_http_client_config_t config_post = {
        .url = "https://1cl4lag6ba.execute-api.us-east-2.amazonaws.com/prod/keglist",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config_post);


    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

////    //PUT
    esp_http_client_set_url(client, "https://1cl4lag6ba.execute-api.us-east-2.amazonaws.com/prod/keglist");
    esp_http_client_set_method(client, HTTP_METHOD_PUT);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP PUT Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP PUT request failed: %s", esp_err_to_name(err));
    }

	esp_http_client_cleanup(client);

}


// Function for Pulse count handler

static bool example_pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    // send event data to queue, from this interrupt callback
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    return (high_task_wakeup == pdTRUE);
}

// GPIO ISR handler and task

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {

        	gpio_intr_disable(25); // Disable GPIO interrupts in case to prevent button bouncing as well as rapid presses


            // Connecting to flow meter
            ESP_LOGI(TAG, "install pcnt unit");
            pcnt_unit_config_t unit_config = {
                .high_limit = EXAMPLE_PCNT_HIGH_LIMIT,
                .low_limit = EXAMPLE_PCNT_LOW_LIMIT,
            };
            pcnt_unit_handle_t pcnt_unit = NULL;
            ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

            ESP_LOGI(TAG, "set glitch filter");
            pcnt_glitch_filter_config_t filter_config = {
                .max_glitch_ns = 10,
            };
            ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

            ESP_LOGI(TAG, "install pcnt channels");
            pcnt_chan_config_t chan_a_config = {
                .edge_gpio_num = EXAMPLE_EC11_GPIO_A,
                .level_gpio_num = -1,
        		.flags.virt_level_io_level=1,
            };
            pcnt_channel_handle_t pcnt_chan_a = NULL;
            ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));

            ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
            ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

            ESP_LOGI(TAG, "add watch points and register callbacks");
            int watch_points[] = {EXAMPLE_PCNT_LOW_LIMIT, -50, 0, 50, EXAMPLE_PCNT_HIGH_LIMIT};
            for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++) {
                ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]));
            }
            pcnt_event_callbacks_t cbs = {
                .on_reach = example_pcnt_on_reach,
            };
            QueueHandle_t queue = xQueueCreate(10, sizeof(int));
            ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

            ESP_LOGI(TAG, "enable pcnt unit");
            ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
            ESP_LOGI(TAG, "clear pcnt unit");
            ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
            ESP_LOGI(TAG, "start pcnt unit");
            ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

            int pulse_count = 0;
            int event_count = 0;

            int sample_time_ms = 100;
            float flow_calibration = 17.2;
            float total = 0;
            int helpervol = selectedvol;
            float to = 0; // temperature of object

            int please=0;

            // Where dispense happens
            //while (total < 0.2)
            //{
            gpio_set_level(GPIO_NUM_16,1); // Set Solenoid GPIO high
            				while (total < helpervol){
            					// usleep(200);
            			        MLX90614_GetTo(MLX90614_DEFAULT_ADDRESS, &to);
            			        printf("log:%lf\n", ((to * 9/5) + 32));

            					if (xQueueReceive(queue, &event_count, pdMS_TO_TICKS(sample_time_ms))) {
            						ESP_LOGI(TAG, "Watch point event, count: %d", event_count);
            					}
            					ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
            					ESP_LOGI(TAG, "Pulse count: %d", pulse_count);
            					float pulse_per_sec = (float) pulse_count / ((float) sample_time_ms / 1000.0);
            					float flow_per_min = pulse_per_sec / flow_calibration;
            					float flow_per_sec = flow_per_min / 60.0;
            					float liters_flown = (float) flow_per_sec * ((float) sample_time_ms / 1000.0);
            					total += liters_flown * 33.814; // The 33.814 converts from liters to ounces
            					ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
            				}
            				gpio_set_level(GPIO_NUM_16,0); // Set Solenoid GPIO low
            				ESP_LOGI(TAG, "TOTAL FLOWN: %lf", total); // Log total volume flown
            		//}

            // Begin taking weight of keg

            HX711_init(GPIO_DATA,GPIO_SCLK,eGAIN_128);
            double weight =1;
            while (please < 10)
            {
                weight = HX711_get_units(AVG_SAMPLES);
                ESP_LOGI(TAG, "******* weight = %f *********\n ", (weight+1.0326)/0.9877);
                vTaskDelay(10 / portTICK_PERIOD_MS);
                please++;
            }

            double wea = (weight+1.0326)/0.9877;
            volleft =( (wea-31.5) * 453.592) /1000;


            	// Temporary values in order to send to eeprom for temperature if needed
           		double tempera = ((double)to * 9/5) + 32;
           		int x = tempera * 100;
           		int firsttemp = x/100;
           		int secondtemp = x-firsttemp*100;

           		// Temporary values in order to send to eeprom for volume if needed
           		int y = volleft * 100;
           		int firstvol = y/100;
           		int secondvol = y-firstvol*100;

    	    	vTaskDelay(1000 / portTICK_PERIOD_MS);

    	    	vTaskDelay(1000 / portTICK_PERIOD_MS);
    	    	vTaskDelay(1000 / portTICK_PERIOD_MS);

    	    	bits = xEventGroupWaitBits(sta_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
    	    		                                    pdFALSE, pdFALSE, portMAX_DELAY);

    	    vTaskDelay(2000 / portTICK_PERIOD_MS);
    	    if (bits & WIFI_CONNECTED_BIT)
    	    {

    	    		vTaskDelay(1000 / portTICK_PERIOD_MS);
    	    		// Format data in order to send to AWS
    	    		char postdata[80];
    	    		sprintf(postdata,"{\"kegId\":\"1\",\"data\":{\"temp\":\"%d.%d\",\"vol\":\"%d.%d\"}}",firsttemp,secondtemp,firstvol,secondvol);
    	    		puts(postdata);
    	    		post_rest_function(postdata);
    	    }
    	   else{
   	    	pointerval++;
   	    	vTaskDelay(1000 / portTICK_PERIOD_MS);
   	    	 // WRITE TEMPERATURE
   	        // EEPROM single byte write first half of temp
   	        uint8_t single_write_byte_pointer = pointerval;
   	        eeprom_write_byte(eeprom_address, starting_address, pointerval);
   	        printf("Wrote byte 0x%02X to address 0x%04X\n", single_write_byte_pointer, starting_address);
   	        vTaskDelay(20/portTICK_PERIOD_MS);

   			// EEPROM single byte write first half of temp
   			uint8_t single_write_byte1 = firsttemp;
   			eeprom_write_byte(eeprom_address, ((starting_address+1)+(pointerval-1)*4), firsttemp);
   			printf("Wrote byte 0x%02X to address 0x%04X\n", single_write_byte1, ((starting_address+1)+(pointerval-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte write second half of temp
   			uint8_t single_write_byte2 = secondtemp;
   			eeprom_write_byte(eeprom_address, ((starting_address+2)+(pointerval-1)*4), secondtemp);
   			printf("Wrote byte 0x%02X to address 0x%04X\n", single_write_byte2, ((starting_address+2)+(pointerval-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);


   			// WRITE VOLUME
   			// EEPROM single byte write first half of vol
   			uint8_t single_write_byte3 = firstvol;
   			eeprom_write_byte(eeprom_address, ((starting_address+3)+(pointerval-1)*4), firstvol);
   			printf("Wrote byte 0x%02X to address 0x%04X\n", single_write_byte3, ((starting_address+3)+(pointerval-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte write second half of vol
   			uint8_t single_write_byte4 = secondvol;
   			eeprom_write_byte(eeprom_address, ((starting_address+4)+(pointerval-1)*4), secondvol);
   			printf("Wrote byte 0x%02X to address 0x%04X\n", single_write_byte4,  ((starting_address+4)+(pointerval-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);


   			// READ
   			// EEPROM single byte read pointer
   			uint8_t random_read_pointer = eeprom_read_byte(eeprom_address, starting_address);
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_pointer, starting_address);
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte read first half of temp
   			uint8_t random_read_byte1 = eeprom_read_byte(eeprom_address, ((starting_address+1)+(pointerval-1)*4));
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte1, ((starting_address+1)+(pointerval-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte read second half of temp
   			uint8_t random_read_byte2 = eeprom_read_byte(eeprom_address, ((starting_address+2)+(pointerval-1)*4));
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte2, ((starting_address+2)+(pointerval-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte read first half of vol
   			uint8_t random_read_byte3 = eeprom_read_byte(eeprom_address, ((starting_address+3)+(pointerval-1)*4));
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte3, ((starting_address+3)+(pointerval-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte read second half of vol
   			uint8_t random_read_byte4 = eeprom_read_byte(eeprom_address,  ((starting_address+4)+(pointerval-1)*4));
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte4,  ((starting_address+4)+(pointerval-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
    	    }


    	    printf("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());
                int id = 1;
                num = ((to * 9/5) + 32);
                ESP_LOGI(TAG, "PRINTING TO SCREEN");

                lcd_clear();

                vTaskDelay(20/portTICK_PERIOD_MS);

                sprintf(buffer, "ID: %d   T:%.2f", id, num);
                lcd_put_cur(0,0);
                lcd_send_string(buffer);
                sprintf(buffer, "VS: %d   V:%.2f", selectedvol, volleft);
                lcd_put_cur(1,0);
                lcd_send_string(buffer);
                gpio_intr_enable(25);

        }
    }
}


void app_main(void)
{
	i2c_master_init();
	vTaskDelay(1000 / portTICK_PERIOD_MS);
    MLX90614_SMBusInit(21, 22, 100000); // sda scl and 50kHz
    lcd_init();

    vTaskDelay(3000 / portTICK_PERIOD_MS);
    lcd_clear();

    lcd_put_cur(0,0);
    lcd_send_string("Welcome!");
    lcd_put_cur(1,0);
    lcd_send_string("Open WIFI");


    vTaskDelay(5000 / portTICK_PERIOD_MS);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    //Create an event group
        sta_wifi_event_group = xEventGroupCreate();

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
        assert(sta_netif);

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        //Registration event handling service
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler,
                                                            NULL, &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler,
                                                            NULL, &instance_got_ip));

        wifi_config_t wifi_config = {
            .sta = {
                .ssid = "Mitchi iPhone12",
                .password = "umez6i4dbeiu3",
                //Equipment from the AP listener sign
                //.listen_interval = DEFAULT_LISTEN_INTERVAL,
                //Setting your password means that the device will connect all security modes, including WEP, WPA. However, these patterns are not recommended, if your access point does not support WPA2, comment to enable other modes
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,

                .pmf_cfg = {
                    .capable = true,
                    .required = false
                },
            },
        };

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        ESP_ERROR_CHECK(esp_wifi_start() );
        ESP_LOGI("WIFI_STA", "esp_wifi_set_ps().");
        //esp_wifi_set_ps(DEFAULT_PS_MODE);
        ESP_LOGI("WIFI_STA", "wifi_init_sta finished.");

    //int w = wifi_init_staa();

    	bits = xEventGroupWaitBits(sta_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                        pdFALSE, pdFALSE, portMAX_DELAY);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    //printf("WIFI was initiated ...........\n\n");

    if (bits & WIFI_CONNECTED_BIT)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        lcd_clear();

        lcd_put_cur(0,0);
        lcd_send_string("Connection");
        lcd_put_cur(1,0);
        lcd_send_string("Success!");
    }
    else{
    	vTaskDelay(1000 / portTICK_PERIOD_MS);
    	lcd_clear();

    	lcd_put_cur(0,0);
    	lcd_send_string("Connection");
    	lcd_put_cur(1,0);
    	lcd_send_string("Failed");
    }




	gpio_set_direction(GPIO_NUM_16,GPIO_MODE_OUTPUT);

	gpio_set_direction(GPIO_NUM_32,GPIO_MODE_INPUT);
	gpio_set_direction(GPIO_NUM_33,GPIO_MODE_INPUT);


    vTaskDelay(3000 / portTICK_PERIOD_MS);

    int id1 = 1;
    num = 00.00;
    selectedvol = 8;
    volleft = 00.00;

    lcd_clear();
    int please=0;
    HX711_init(GPIO_DATA,GPIO_SCLK,eGAIN_128);
    double weight =1;
    while (please < 10)
    {
       weight = HX711_get_units(AVG_SAMPLES);
       ESP_LOGI(TAG, "******* weight = %f *********\n ", (weight+1.0326)/0.9877);
       vTaskDelay(10 / portTICK_PERIOD_MS);
       please++;
    }

    double wea = (weight+1.0326)/0.9877;
    volleft =( (wea-31.5) * 453.592) /1000;


    num = 72.18;

    sprintf(buffer, "ID: %d   T:%.2f", id1,num);
    lcd_put_cur(0,0);
    lcd_send_string(buffer);
    sprintf(buffer, "VS: %d   V:%.2f", selectedvol, volleft);
    lcd_put_cur(1,0);
    lcd_send_string(buffer);






//    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
//    //disable interrupt
//    io_conf.intr_type = GPIO_INTR_DISABLE;
//    //set as output mode
//    io_conf.mode = GPIO_MODE_OUTPUT;
//    //bit mask of the pins that you want to set,e.g.GPIO18/19
//    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
//    //disable pull-down mode
//    io_conf.pull_down_en = 0;
//    //disable pull-up mode
//    io_conf.pull_up_en = 0;
//    //configure GPIO with the given settings
//    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-down mode
    io_conf.pull_up_en = 1;

    gpio_config(&io_conf);

    //change gpio interrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_POSEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 8000, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

    //remove isr handler for gpio number.
    gpio_isr_handler_remove(GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin again
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

	pointerval = eeprom_read_byte(eeprom_address, starting_address);
	printf("Read byte 0x%02X at address 0x%04X\n", pointerval, starting_address);
	vTaskDelay(20/portTICK_PERIOD_MS);

    printf("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());

    int temper=selectedvol;

    while(1) {
    	bits = xEventGroupWaitBits(sta_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
    		                                    pdFALSE, pdFALSE, portMAX_DELAY);
        if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI("WIFI_STA", "Failed to connect, restarting station");
            ESP_ERROR_CHECK(esp_wifi_stop());
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            ESP_ERROR_CHECK(esp_wifi_start());
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        };

        if (gpio_get_level(33) == 1){
        	selectedvol++;
        }
        else if(gpio_get_level(32) == 1){
        	if (selectedvol == 1){

        	}
        	else{
        		selectedvol--;
        	}
        }
        else{
        	if ((bits & WIFI_CONNECTED_BIT) && pointerval>0){
     	       vTaskDelay(1000 / portTICK_PERIOD_MS);
     	        lcd_clear();

     	        lcd_put_cur(0,0);
     	        lcd_send_string("Busy");
     	        lcd_put_cur(1,0);
     	        lcd_send_string("Wait!");
     	       int x;
     	       int temppointer = pointerval;
     	       for(x=1;x<=temppointer;x++){
     	    	  gpio_intr_disable(25); // Disable GPIO interrupts in case to prevent button bouncing as well as rapid presses
     	       // Format data in order to send to AWS
   			// EEPROM single byte read first half of temp
   			uint8_t random_read_byte1 = eeprom_read_byte(eeprom_address, ((starting_address+1)+(x-1)*4));
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte1, ((starting_address+1)+(x-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte read second half of temp
   			uint8_t random_read_byte2 = eeprom_read_byte(eeprom_address, ((starting_address+2)+(x-1)*4));
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte2, ((starting_address+2)+(x-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte read first half of vol
   			uint8_t random_read_byte3 = eeprom_read_byte(eeprom_address, ((starting_address+3)+(x-1)*4));
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte3, ((starting_address+3)+(x-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
   			// EEPROM single byte read second half of vol
   			uint8_t random_read_byte4 = eeprom_read_byte(eeprom_address,  ((starting_address+4)+(x-1)*4));
   			printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte4,  ((starting_address+4)+(x-1)*4));
   			vTaskDelay(20/portTICK_PERIOD_MS);
     	       char postdata[80];
     	       sprintf(postdata,"{\"kegId\":\"1\",\"data\":{\"temp\":\"%d.%d\",\"vol\":\"%d.%d\"}}",random_read_byte1,random_read_byte2,random_read_byte3,random_read_byte4);
     	       puts(postdata);
     	      vTaskDelay(1000/portTICK_PERIOD_MS);
     	       post_rest_function(postdata);
     	      printf("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());
     	       pointerval--;
      	        uint8_t single_write_byte_pointer = pointerval;
      	        eeprom_write_byte(eeprom_address, starting_address, pointerval);
      	        printf("Wrote byte 0x%02X to address 0x%04X\n", single_write_byte_pointer, starting_address);
      	        vTaskDelay(20/portTICK_PERIOD_MS);
     	      gpio_intr_enable(25);
     	       }
               lcd_clear();
               sprintf(buffer, "ID: %d   T:%.2f", id1, num);
               lcd_put_cur(0,0);
               lcd_send_string(buffer);
               sprintf(buffer, "VS: %d   V:%.2f", selectedvol, volleft);
               lcd_put_cur(1,0);
               lcd_send_string(buffer);
               vTaskDelay(500 / portTICK_PERIOD_MS);
        	}

        }

        if(selectedvol != temper){

        lcd_clear();
        sprintf(buffer, "ID: %d   T:%.2f", id1, num);
        lcd_put_cur(0,0);
        lcd_send_string(buffer);
        sprintf(buffer, "VS: %d   V:%.2f", selectedvol, volleft);
        lcd_put_cur(1,0);
        lcd_send_string(buffer);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        else{

        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
        temper = selectedvol;
    }
}
