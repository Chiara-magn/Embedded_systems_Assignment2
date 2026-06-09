#include "IMU_handler.h"
#include "SPI_handler.h"
#include "UART_handler.h"
#include "config.h"
#include "timer.h"

static float yaw_deg_gyro = 0.0f;

// Initializes SPI settings for IMU communication
void imu_setup(void) {
    // Bosch BMX055 SPI protocol require idle LOW
    SPI1STATbits.SPIEN = 0;     // disable SPI to change settings
    SPI1CON1bits.CKE = 1;       // Output data changes on transition from active to idle
    SPI1CON1bits.CKP = 0;       // Idle state for clock is a low level
    SPI1STATbits.SPIEN = 1;     // re-enable SPI
    tmr_wait_ms(TIMER2, 10);
}

/*
  Initialize all three IMU sensors and verify chip IDs.
  Magnetometer starts in sleep mode and must be explicitly woken up, 
  Accelerometer and Gyroscope start in normal mode.

  Then we checks IDs to see proper initialization and we 
  set active mode for all, default bandwidth for the accelerometer 
  is 1000 Hz (value 15 in register 0x10)
 */


void imu_init(void) {

    uart_send_string("IMU INIT START\r\n");

    // Deselect all sensors (CS idle = high)
    ACC_CS_LAT = 1;
    GYR_CS_LAT = 1;
    MAG_CS_LAT = 1;

     // Magnetometer starts in suspend mode it must be woken up explicitly
    imu_write_register(IMU_MAG, 0x4B, 0x01);  // Enter sleep mode first
    tmr_wait_ms(TIMER1, 10);                   // Wait for mode transition
    imu_write_register(IMU_MAG, 0x4C, 0x00);   // Enter active mode
    tmr_wait_ms(TIMER1, 10);                   // Wait for stabilization

    // Debug: read and print chip IDs to verify communication
    uint8_t ACC_ID = imu_read_chip_id(IMU_ACC);
    uint8_t GYR_ID = imu_read_chip_id(IMU_GYR);
    uint8_t MAG_ID = imu_read_chip_id(IMU_MAG);


    // check accelerometer ID from datasheet 1111 1010 -> 0xFA
     if (ACC_ID != ACC_CHIP_ID) {
        uart_send_string("Incorrect accelerometer Chip ID\r\n");
        return; // Abort initialization
    }

    // check gyroscope ID from datasheet 0000 1111 -> 0x0F
    if (GYR_ID != GYR_CHIP_ID) {
        uart_send_string("Incorrect gyroscope Chip ID\r\n");
        return;
    }

    // check magnetometer ID from datasheet -> 0x32
    if (MAG_ID != MAG_CHIP_ID) {
        uart_send_string("Incorrect magnetometer Chip ID\r\n");
        return;
    } 
    
    // if we reach this point, all IDs are correct
    uart_send_string("All Chip IDs are CORRECT!\r\n");

    // Set gyroscope measurement range to ±125°/s (register 0x0F, value 0x04)
    // Narrower range = higher resolution. Suitable for a slow-moving buggy; change to 0x03 (±250°/s) if needed
    imu_write_register(IMU_GYR, 0x0F, 0x04); 
    tmr_wait_ms(TIMER1, 5);


    // Readback verification: re-read register 0x0F and print via UART
    // Expected output: "GYR_RANGE: 0x04" — if different, SPI write failed
    uint8_t gyr_range = imu_read_register(IMU_GYR, 0x0F);
    char msg[30];
    sprintf(msg, "GYR_RANGE: 0x%02X\r\n", gyr_range);
    uart_send_string(msg); 

    // Set accelerometer internal filter bandwidth to 1000 Hz (register 0x10, value 15)
    // Highest bandwidth = minimum filtering = fastest response to movement
    imu_set_bandwidth(15);  // 1000 Hz default
}


/*
  Select one IMU sensor by pulling its CS low
  Always deselects all sensors first to avoid bus conflicts
  dev -> Sensor to select (IMU_ACC, IMU_GYR, IMU_MAG)
 */

static void imu_select(imu_device_t dev)  
{
    // Deselect all sensors first (CS idle = high)
    ACC_CS_LAT = 1;
    GYR_CS_LAT = 1;
    MAG_CS_LAT = 1;

    // activate only the selected sensor (CS active = low)
    switch (dev) {
        case IMU_ACC: ACC_CS_LAT = 0; break;
        case IMU_GYR: GYR_CS_LAT = 0; break;
        case IMU_MAG: MAG_CS_LAT = 0; break;
    }
}

/*
 Write one byte to a sensor register via SPI
  dev ->  Target sensor
  reg -> Register address (MSB will be forced to 0 = write)
  value -> Value to write
 */

void imu_write_register(imu_device_t dev, uint8_t reg, uint8_t value)
{
    imu_select(dev);
    spi_write(reg & 0x7F);   // write -> MSB = 0
    spi_write(value);
    // deselect
    ACC_CS_LAT = 1;
    GYR_CS_LAT = 1;
    MAG_CS_LAT = 1;
}

/*
  Read one byte from a sensor register via SPI
  dev -> Target sensor
  reg -> Register address (MSB will be forced to 1 = read)
  return Register value
 */

uint8_t imu_read_register(imu_device_t dev, uint8_t reg)
{
    imu_select(dev);
    spi_write(reg | 0x80);   // MSB=1 -> read operation
    uint8_t value = spi_write(0x00); // Clock out zeros to receive data
    // deselect
    ACC_CS_LAT = 1;
    GYR_CS_LAT = 1;
    MAG_CS_LAT = 1;
    return value;
}

/*
  Read chip ID from a sensor
  Magnetometer chip ID is at register 0x40, others at 0x00
  dev -> Target sensor
  return Chip ID byte
 */

uint8_t imu_read_chip_id(imu_device_t dev)
{
    if (dev == IMU_MAG)
        return imu_read_register(dev, 0x40);
    else
        return imu_read_register(dev, 0x00);
}

/*
  imu_read_mag - Read raw magnetometer data (BMM150)
  Read 6 bytes starting from register 0x42 (DATAX_LSB).
  X and Y are 13-bit signed values: mask LSB with 0xF8, shift right by 3.
  Z is 15-bit signed: mask LSB with 0xFE, shift right by 1.
  All CS lines are put HIGH after the transaction.
 */

void imu_read_mag(sensor_data_t *data)
{
    uint8_t buf[6];
    
    imu_select(IMU_MAG);
    spi_write(0x42 | 0x80);  // address | read bit (1)
    buf[0] = spi_write(0x00);  // X LSB
    buf[1] = spi_write(0x00);  // X MSB
    buf[2] = spi_write(0x00);  // Y LSB
    buf[3] = spi_write(0x00);  // Y MSB
    buf[4] = spi_write(0x00);  // Z LSB
    buf[5] = spi_write(0x00);  // Z MSB
    ACC_CS_LAT = 1;
    GYR_CS_LAT = 1;
    MAG_CS_LAT = 1;

    data->x = ((int16_t)buf[1] << 8 | (buf[0] & 0xF8)) >> 3;
    data->y = ((int16_t)buf[3] << 8 | (buf[2] & 0xF8)) >> 3;
    data->z = ((int16_t)buf[5] << 8 | (buf[4] & 0xFE)) >> 1;
}

/*
  imu_read_acc - Read raw accelerometer data (BMA280)
  Read 6 bytes starting from register 0x02.
  Each axis is a 12-bit signed value: mask LSB with 0xF0, shift right by 4.
  All CS lines are put HIGH after the transaction.
 */

void imu_read_acc(sensor_data_t *data)
{
    uint8_t buf[6];
    
    imu_select(IMU_ACC);
    spi_write(0x02 | 0x80);  // address | read bit (1)
    buf[0] = spi_write(0x00);  // X LSB
    buf[1] = spi_write(0x00);  // X MSB
    buf[2] = spi_write(0x00);  // Y LSB
    buf[3] = spi_write(0x00);  // Y MSB
    buf[4] = spi_write(0x00);  // Z LSB
    buf[5] = spi_write(0x00);  // Z MSB
    ACC_CS_LAT = 1;
    GYR_CS_LAT = 1;
    MAG_CS_LAT = 1;

    data->x = ((int16_t)buf[1] << 8 | (buf[0] & 0xF0)) >> 4;
    data->y = ((int16_t)buf[3] << 8 | (buf[2] & 0xF0)) >> 4;
    data->z = ((int16_t)buf[5] << 8 | (buf[4] & 0xF0)) >> 4;
}

/*
 * imu_read_gyro - Read raw gyroscope data (BMG160)
 * Read 2 registers per axis (LSB then MSB) for X, Y, Z sequentially.
 * Each axis is a 16-bit signed value, no masking needed (full resolution).
 * With range set to ±125°/s: angular rate [°/s] = raw / 262.4
 */

 /*void imu_read_gyro(sensor_data_t *data)
{
    uint8_t lsb, msb;

    // X axis
    lsb = imu_read_register(IMU_GYR, 0x02);
    msb = imu_read_register(IMU_GYR, 0x03);
    data->x = (int16_t)((msb << 8) | lsb);

    // Y axis
    lsb = imu_read_register(IMU_GYR, 0x04);
    msb = imu_read_register(IMU_GYR, 0x05);
    data->y = (int16_t)((msb << 8) | lsb);

    // Z axis (yaw rate)
    lsb = imu_read_register(IMU_GYR, 0x06);
    msb = imu_read_register(IMU_GYR, 0x07);
    data->z = (int16_t)((msb << 8) | lsb);
}*/

void imu_read_gyro(sensor_data_t *data)
{
    uint8_t buf[6];

    imu_select(IMU_GYR);
    spi_write(0x02 | 0x80);  // address | read bit (1)
    buf[0] = spi_write(0x00);  // X LSB
    buf[1] = spi_write(0x00);  // X MSB
    buf[2] = spi_write(0x00);  // Y LSB
    buf[3] = spi_write(0x00);  // Y MSB
    buf[4] = spi_write(0x00);  // Z LSB
    buf[5] = spi_write(0x00);  // Z MSB
    ACC_CS_LAT = 1;
    GYR_CS_LAT = 1;
    MAG_CS_LAT = 1;

    // Gyro is full 16-bit, no masking needed
    data->x = (int16_t)(buf[1] << 8 | buf[0]);
    data->y = (int16_t)(buf[3] << 8 | buf[2]);
    data->z = (int16_t)(buf[5] << 8 | buf[4]);
}

/*
  imu_update_yaw - Integrate gyroscope Z axis to estimate yaw angle.
  Called every IMU_DT = 2 ms (500 Hz task).
  Converts raw values to °/s using range divisor 262.4 (±125°/s setting),
  then accumulates: yaw += gyro_z [°/s] * dt [s].
  Note: simple Euler integration — drift accumulates over time. Suitable for short maneuvers .
 */

void imu_update_yaw(void)
{
    sensor_data_t gyro;
    imu_read_gyro(&gyro); // pointer to struct where gyro data will be stored

    float gyro_z_dps = gyro.z / 262.4f; // Convert raw Z to degrees per second (°/s) using range divisor for ±125°/s

    // euler integration to update yaw angle: yaw += angular_rate * dt
    yaw_deg_gyro += gyro_z_dps * IMU_DT;
}


void imu_reset_yaw_gyro(void)
{
    yaw_deg_gyro = 0.0f;
}

float imu_get_yaw_gyro(void)
{
    return yaw_deg_gyro;
}


/*
  Set accelerometer low-pass filter bandwidth
  Written to register 0x10
  8=7.81Hz, 9=15.63Hz, 10=31.25Hz, 11=62.5Hz,
  12=125Hz, 13=250Hz, 14=500Hz, 15=1000Hz
  bandwidth_value -> Value between 8 and 15
 */

void imu_set_bandwidth(uint8_t bandwidth_value)
{
    imu_write_register(IMU_ACC, 0x10, bandwidth_value);
}


/*
  imu_roll_pitch_yaw - Compute roll, pitch, and tilt-compensated yaw.
 
  Roll and pitch are derived from the accelerometer only, using gravity
  projection on the sensor axes:
    roll  = atan2(ay, az)
    pitch = atan2(-ax, sqrt(ay^2 + az^2))  [sqrt for stability near ±90°]
 
  Yaw requires the magnetometer. Since the board may be tilted, raw mag_y/mag_x
  would give a wrong heading — the magnetic field is rotated back to horizontal
  using roll and pitch (tilt compensation) before calling atan2.
 
  Output angles are in degrees. Yaw is relative to magnetic north.
  Note: accelerometer-based roll/pitch is noisy under vibration/acceleration.
  For dynamic use, consider a complementary or Kalman filter.
 */

void imu_roll_pitch_yaw(const sensor_data_t *acc, const sensor_data_t *mag, angle_data_t *angles)
{
    float ax = (float)acc->x;
    float ay = (float)acc->y;
    float az = (float)acc->z;

    // Roll: rotation around X axis
    angles->roll  = atan2f(ay, az) * RAD_TO_DEG;

    // Pitch: rotation around Y axis
    // Uses sqrt of ay²+az² as denominator for stability near ±90°
    angles->pitch = atan2f(-ax, sqrtf(ay*ay + az*az)) * RAD_TO_DEG;

    // Yaw: rotation around Z axis (requires magnetometer)
    float mx = (float)mag->x;
    float my = (float)mag->y;
    float mz = (float)mag->z;

    // Convert roll and pitch to radians for tilt compensation
    float roll_rad  = angles->roll  * DEG_TO_RAD;
    float pitch_rad = angles->pitch * DEG_TO_RAD;

    // Tilt-compensated magnetic field components
    float cos_roll  = cosf(roll_rad);
    float sin_roll  = sinf(roll_rad);
    float cos_pitch = cosf(pitch_rad);
    float sin_pitch = sinf(pitch_rad);

    float mag_x = mx * cos_pitch
                + my * sin_roll * sin_pitch
                + mz * cos_roll * sin_pitch;

    float mag_y = my * cos_roll
                - mz * sin_roll;

    // Yaw from tilt-compensated magnetic north
    angles->yaw = atan2f(-mag_y, mag_x) * RAD_TO_DEG;
}