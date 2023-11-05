#include <stdlib.h>

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

}
