#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-fpermissive"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "controller.h"
#include "bt.h"
#include "bt_trace.h"
#include "bt_types.h"
#include "btm_api.h"
#include "bta_api.h"
#include "bta_gatt_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"

#pragma GCC diagnostic pop

#define GATTS_SERVICE_UUID_TEST_LED   0xAABB

//Declare callback functions
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

/* name of BLE device that you want to connect to */
static const char device_name[] = "ESP_GATTS_IOTSHARING";
static bool connect = false;

// from Arduino Serial Monitor type 'on' to turn on led, 'off' to off LED
String command = "";

// BLE scan params
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30
};

// profile info
#define PROFILE_ON_APP_ID 0

//we have 2 characteristics
#define CHAR_NUM 2
// index of characteristics in array for led ctrl and temp notify
#define CHARACTERISTIC_LED_CTRL_ID    0
#define CHARACTERISTIC_TEMP_NOTI_ID   1

//this structure holds the information of GATT profile
struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t char_handle[CHAR_NUM];
    esp_bd_addr_t remote_bda;
};

// this variable holds the information of GATT profile
static struct gattc_profile_inst test_profile = {
    .gattc_cb = gattc_profile_event_handler,
    .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
};

/* 
This function write value to specific characteristic,
or sending WRITE request to GATT Server
*/
void set_led(uint8_t val){
    uint8_t state = val;
    esp_ble_gattc_write_char(test_profile.gattc_if,
                            test_profile.conn_id,
                            test_profile.char_handle[CHARACTERISTIC_LED_CTRL_ID],
                            sizeof(state),
                            &state,
                            ESP_GATT_WRITE_TYPE_RSP,
                            ESP_GATT_AUTH_REQ_NONE);
}
/* 
This callback will will be invoked when GATT BLE events come.
Refer GATT Client callback function events here: 
https://github.com/espressif/esp-idf/blob/master/components/bt/bluedroid/api/include/esp_gattc_api.h
*/
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    uint16_t conn_id = 0;
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
    switch (event) {
        case ESP_GATTC_REG_EVT:{
            esp_ble_gap_set_scan_params(&ble_scan_params);
            Serial.printf("\nESP_GATTC_REG_EVT\n");
            break;
        }
        /* when connection is set up, the event comes */
        case ESP_GATTC_OPEN_EVT: {
            conn_id = p_data->open.conn_id;
            memcpy(test_profile.remote_bda, p_data->open.remote_bda, sizeof(esp_bd_addr_t));
            esp_ble_gattc_search_service(gattc_if, conn_id, NULL);
            test_profile.gattc_if = gattc_if;
            test_profile.conn_id = conn_id;
            Serial.printf("\nESP_GATTC_OPEN_EVT\n");
            break;
        }
        // When GATT service discovery result is got, the event comes
        case ESP_GATTC_SEARCH_RES_EVT: {
            esp_gatt_srvc_id_t *srvc_id = (esp_gatt_srvc_id_t *)&p_data->search_res.srvc_id;
            conn_id = p_data->search_res.conn_id;
   // The service may have many characteristics but we only care GATTS_SERVICE_UUID_TEST_LED
            if (srvc_id->id.uuid.len == ESP_UUID_LEN_16 && srvc_id->id.uuid.uuid.uuid16 == GATTS_SERVICE_UUID_TEST_LED) {
                Serial.printf("\nservice UUID16: %x\n", srvc_id->id.uuid.uuid.uuid16);
    // store start_handle and end_handle for later usage
                test_profile.start_handle = p_data->search_res.start_handle;
                test_profile.end_handle = p_data->search_res.end_handle;
            }
            Serial.printf("\nESP_GATTC_SEARCH_RES_EVT RES\n");
            break;
        }
  // When GATT service discovery is completed, the event comes
        case ESP_GATTC_SEARCH_CMPL_EVT: {
   // store connection id for later usage
            conn_id = p_data->search_cmpl.conn_id;
            esp_gattc_char_elem_t *char_elements;
            uint16_t char_offset = 0;
            uint16_t count = 0;
   // get characteristics of service. It just get characteristic from local cache, won't get from remote devices
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count( test_profile.gattc_if,
                                           test_profile.conn_id,
                                           ESP_GATT_DB_CHARACTERISTIC,
                                           test_profile.start_handle,
                                           test_profile.end_handle,
                                           ESP_GATT_INVALID_HANDLE,
                                           &count);
            if (count > 0){
                char_elements = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                status = esp_ble_gattc_get_all_char(test_profile.gattc_if, 
                                          test_profile.conn_id, 
                                          test_profile.start_handle, 
                                          test_profile.end_handle, 
                                          char_elements, &count, char_offset);                                     
                for(int i=0; i<count; i++){
                    Serial.printf("\n%d) char uuid = %x, char_handle = %d\n", i, char_elements[i].uuid.uuid.uuid16, char_elements[i].char_handle);
                }
    // store characteristic handles of CHARACTERISTIC_LED_CTRL_ID and CHARACTERISTIC_TEMP_NOTI_ID for later usage
                test_profile.char_handle[CHARACTERISTIC_LED_CTRL_ID] = char_elements[CHARACTERISTIC_LED_CTRL_ID].char_handle;
                test_profile.char_handle[CHARACTERISTIC_TEMP_NOTI_ID] = char_elements[CHARACTERISTIC_TEMP_NOTI_ID].char_handle;
                /* 
    We use BLE notification to monitor temperature of GATT server.
    So we check if temp noti bit was set we will register to receive notification from GATT server
    */
                if (char_elements[CHARACTERISTIC_TEMP_NOTI_ID].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
                    Serial.printf("\nregister noti\n");
                    //if it set then we enable noti, the event ESP_GATTC_REG_FOR_NOTIFY_EVT will be raised when finishing
                    esp_ble_gattc_register_for_notify (gattc_if, test_profile.remote_bda, test_profile.char_handle[CHARACTERISTIC_TEMP_NOTI_ID]);
                }
                free(char_elements);
            }
            Serial.printf("\nESP_GATTC_SEARCH_CMPL_EVT\n");
            break;
        }
  // When register for notification of a service completes, the event comes
        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
            Serial.printf("\nESP_GATTC_REG_FOR_NOTIFY_EVT\n");
            break;
        }
        // when central device send data to this device, this event will be invoked and we print the temp data
        case ESP_GATTC_NOTIFY_EVT: {
            Serial.printf("\nESP_GATTC_NOTIFY_EVT\n");
            if(p_data->notify.value_len == 1){
                Serial.printf("\ntemperature from noti %d\n", p_data->notify.value[0]);
            }
            break;
        }
        default:
            break;
    }
}

/* 
This callback will be invoked when GAP advertising events come.
Refer GAP BLE callback event type here: 
https://github.com/espressif/esp-idf/blob/master/components/bt/bluedroid/api/include/esp_gap_ble_api.h
*/
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        Serial.printf("\nESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT\n");
        //the unit of the duration is second
        uint32_t duration = 30;
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        Serial.printf("\nESP_GAP_BLE_SCAN_START_COMPLETE_EVT\n");
        //scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            Serial.printf("\nScan start failed");
        }
        break;
    /* processing scan result */
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            Serial.printf("\nESP_GAP_BLE_SCAN_RESULT_EVT\n");
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            if (adv_name != NULL) {
                if (strlen(device_name) == adv_name_len && strncmp((char *)adv_name, device_name, adv_name_len) == 0) {
                    /* if connection is established then stop scanning */
                    if (connect == false) {
                        connect = true;
                        Serial.printf("\nConnect to the remote device %s\n", device_name);
                        esp_ble_gap_stop_scanning();
                        esp_ble_gattc_open(test_profile.gattc_if, scan_result->scan_rst.bda, true);
                    }
                }
            }
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            Serial.printf("\nESP_GAP_SEARCH_INQ_CMPL_EVT\n");
            break;
        default:
            break;
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        Serial.printf("\nESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT\n");
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            Serial.printf("\nScan stop failed\n");
        } else {
            Serial.printf("\nStop scan successfully\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        Serial.printf("\nESP_GAP_BLE_ADV_STOP_COMPLETE_EVT\n");
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            Serial.printf("\nAdv stop failed\n");
        } else {
            Serial.printf("\nStop adv successfully");
        }
        break;
    default:
        break;
    }
}

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            test_profile.gattc_if = gattc_if;
        } else {
            Serial.printf("\nReg app failed, app_id %04x, status %d\n",
                    param->reg.app_id, 
                    param->reg.status);
            return;
        }
    }
    do {
        /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */  
        if (gattc_if == ESP_GATT_IF_NONE || gattc_if == test_profile.gattc_if) {
            if (test_profile.gattc_cb) {
                gattc_profile_event_handler(event, gattc_if, param);
            }
        }
    } while (0);
}

void ble_client_appRegister(void)
{
    esp_err_t status;
 
    Serial.printf("\nregister callback");
    //register the scan callback function to the gap module
    if ((status = esp_ble_gap_register_callback(esp_gap_cb)) != ESP_OK) {
        Serial.printf("\ngap register error, error code = %x\n", status);
        return;
    }
    //register the callback function to the gattc module
    if ((status = esp_ble_gattc_register_callback(esp_gattc_cb)) != ESP_OK) {
        Serial.printf("\ngattc register error, error code = %x\n", status);
        return;
    }
    esp_ble_gattc_app_register(PROFILE_ON_APP_ID);
}

void gattc_client_test(void)
{
    esp_bluedroid_init();
    esp_bluedroid_enable();
    ble_client_appRegister();
}

void setup(){
  Serial.begin(115200);
  btStart();
  gattc_client_test();
}
/* processing Terminal command, type 'on' to turn on led, 'off' to off LED */
void loop(){
  if(Serial.available() > 0){
      while(Serial.available()){
          command += (char)Serial.read();
          if(command == "on"){
              Serial.println("ON");
              set_led(1);
          }else if(command == "off"){
              Serial.println("OFF");
              set_led(0);
          }
      }
      command = "";
  }
}
