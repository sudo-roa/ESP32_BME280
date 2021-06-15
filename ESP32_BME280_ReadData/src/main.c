// #screen /dev/ttyUSB0 115200

#include "driver/i2c.h"
#include "esp_err.h"

#define WRITE_BIT I2C_MASTER_WRITE
#define READ_BIT I2C_MASTER_READ
#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1
#define STATUS_P_DA (1 << 1)
#define STATUS_T_DA (1 << 0)
// #define CHIP_ADDR 0x77 
#define CHIP_ADDR 0x76              //if jumper from SDO to GND 
#define ID_REGISTER 0xD0            //read only
#define RESET_REGISTER 0xE0         //write only
#define CTRL_HUM_REGISTER 0xF2      
#define STATUS_REGISTER 0xF3
#define CTRL_MEAS_REGISTER 0xF4
#define CONFIG_REGISTER 0xF5
#define PRESS_MSB_REGISTER 0xF7     //read only
#define PRESS_LSB_REGISTER 0xF8     //read only
#define PRESS_XLSB_REGISTER 0xF9    //read only
#define TEMP_MSB_REGISTER 0xFA      //read only
#define TEMP_LSB_REGISTER 0xFB      //read only
#define TEMP_XLSB_REGISTER 0xFC     //read only
#define HUM_MSB_REGISTER 0xFD       //read only
#define HUM_LSB_REGISTER 0xFE       //read only

static esp_err_t i2c_master_driver_initialize(void){
    printf("start initialization...\n");
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = 22,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 200000,
    };
    printf("finish initialization!!\n");
    return i2c_param_config(I2C_NUM_0, &conf);
}

static esp_err_t i2c_set_register(int chip_addr, uint8_t data_pointer, uint8_t data){
    esp_err_t ret;
    printf("set register no.%d\n", data_pointer);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data_pointer, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    printf("finish set reg!!\n");
    return ret;
}

static esp_err_t i2c_read_register(int chip_addr, uint8_t data_pointer, uint8_t* msb_data){
    esp_err_t ret;
    printf("read register no.%02x\n", data_pointer);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data_pointer, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, msb_data, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    printf("finish read reg!!\n");
    return ret;
}

void bme280_read_task(void* pvParameter){
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    i2c_master_driver_initialize();
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    uint8_t registerValue;

    for(;;){
        vTaskDelay(pdMS_TO_TICKS(3000));
        i2c_read_register(CHIP_ADDR, ID_REGISTER, &registerValue);
        printf("reg no.%02x | %02x\n",ID_REGISTER , registerValue);
        i2c_read_register(CHIP_ADDR, STATUS_REGISTER, &registerValue);
        printf("reg no.%02x | %02x\n", STATUS_REGISTER, registerValue);
        i2c_read_register(CHIP_ADDR, CONFIG_REGISTER, &registerValue);
        printf("reg no.%02x | %02x\n", CONFIG_REGISTER, registerValue);
    }
}

void app_main(void){
    xTaskCreate(&bme280_read_task, "bme280_read_task", 8192, NULL, 5, NULL);
}
