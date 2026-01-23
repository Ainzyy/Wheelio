#ifndef MPU6050_MOCK_H
#define MPU6050_MOCK_H

class MPU6050_Mock {
public:
    void initialize() {
        // Mock initialization logic
    }

    void getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
        // Mock data retrieval logic
        if (ax) *ax = 0;
        if (ay) *ay = 0;
        if (az) *az = 0;
        if (gx) *gx = 0;
        if (gy) *gy = 0;
        if (gz) *gz = 0;
    }
};

#endif // MPU6050_MOCK_H