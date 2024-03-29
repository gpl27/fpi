#ifndef IMGPROC_H
#define IMGPROC_H

void rgb_to_l(unsigned char* data, int x, int y, int n);
void vflip(unsigned char* data, int x, int y, int n);
void hflip(unsigned char *data, int x, int y, int n);
void l_quantize(unsigned char *data, int x, int y, int n, int q);


#ifdef IMGPROC_IMPLEMENTATION
#include <stdlib.h>
#include <math.h>

#define map(i, j, k, x, n) (i*x*n + j*n + k)

void rgb_to_l(unsigned char* data, int x, int y, int n) {
    int Ri, Gi, Bi;
    unsigned char L;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            Ri = map(i,j,0,x,n);
            Gi = map(i,j,1,x,n);
            Bi = map(i,j,2,x,n);
            L = (unsigned char) (0.299*data[Ri] + 0.587*data[Gi] + 0.114*data[Bi]);
            memset(&data[Ri], L, n);
        }
    }
}

void vflip(unsigned char* data, int x, int y, int n) {
    char *tmp = malloc(x*n);
    for (int i = 0; i < (int) (y/2); i ++) {
        memcpy(tmp, data + i*x*n, x*n);
        memcpy(data + i*x*n, data + (y-i-1)*x*n, x*n);
        memcpy(data + (y-i-1)*x*n, tmp, x*n);
    }
    free(tmp);
}

void hflip(unsigned char *data, int x, int y, int n) {
    char *tmp = malloc(n);
    for (int j = 0; j < (int) (x/2) ; j++) {
        for (int i = 0; i < y; i++) {
            memcpy(tmp, data + map(i,j,0,x,n), n);
            memcpy(data + map(i,j,0,x,n), data + map(i,(x-j),0,x,n), n);
            memcpy(data + map(i,(x-j),0,x,n), tmp, n);
        }
    }
    free(tmp);
}

void l_quantize(unsigned char *data, int x, int y, int n, int q) {
    // Find t1 and t2 (min and max)
    int t1 = data[0];
    int t2 = data[0];
    for (int i = 0; i < x*y*n; i += n) {
        t1 = (data[i] < t1)? data[i] : t1; // min
        t2 = (data[i] > t2)? data[i] : t2; // max
    }
    int int_size = t2 - t1 + 1;
    if (q >= int_size)
        return; // No quantization is necessary
    
    // Bin and quantize
    float bin_size = (float) int_size / (float) q;
    int L, bin_id;
    float Li, Lj;
    for (int i = 0; i < x*y*n; i += n) {
        bin_id = (data[i] - t1) / bin_size;
        Li = t1 + bin_id * bin_size;
        Lj = t1 + (bin_id+1) * bin_size;
        L = round((double)(Li + Lj)/2);
        memset(&data[i], L, n);
    }
}

#endif

#endif
