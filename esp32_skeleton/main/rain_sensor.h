#ifndef RAIN_SENSOR_H
#define RAIN_SENSOR_H

void rain_sensor_init(void);
int rain_sensor_read(float *level);

#endif // RAIN_SENSOR_H
