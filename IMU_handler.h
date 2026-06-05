#ifndef IMU_HANDLER_H
#define IMU_HANDLER_H
#include <stdint.h> 
#include <math.h>

// Enum identifying which IMU sensor to address in read/write operations
typedef enum {
    IMU_ACC,
    IMU_GYR,
    IMU_MAG
} imu_device_t;

// Raw 3-axis data from the accelerometer or gyroscope or magnetometer
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
}sensor_data_t;


// Orientation angles computed from accelerometer and magnetometer data in degrees
typedef struct {
    float roll;
    float pitch;
    float yaw;
} angle_data_t;


#define RAD_TO_DEG (180.0f / M_PI)
#define DEG_TO_RAD (M_PI / 180.0f)

void imu_init(void);
void imu_setup(void);
void imu_write_register(imu_device_t dev, uint8_t reg, uint8_t value);
uint8_t imu_read_register(imu_device_t dev, uint8_t reg);
uint8_t imu_read_chip_id(imu_device_t dev);

void imu_set_sleep(imu_device_t dev);
void imu_set_active(imu_device_t dev);

// functions Assignment1 
void imu_read_acc(sensor_data_t *data);
void imu_read_mag(sensor_data_t *data);
void imu_read_gyro(sensor_data_t *data);
void imu_update_yaw_gyro(void);
void imu_reset_yaw_gyro(void);
float imu_get_yaw_gyro(void);

void imu_set_bandwidth(uint8_t bandwidth_value);
void imu_roll_pitch_yaw(const sensor_data_t *acc, const sensor_data_t *mag, angle_data_t *angles);

#endif