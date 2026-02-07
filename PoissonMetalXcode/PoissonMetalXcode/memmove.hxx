float* allocate(int n);
void deallocate(float *&d);

void copyDeviceToHost
        (float *h, float *d, int n);
        
void copyHostToDevice
        (float *h, float *d, int n);

void copyDeviceToDevice
        (float *d_out, float *d_in, int n);
