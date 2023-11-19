#ifndef IMGPROC_H
#define IMGPROC_H

#include <gdk/gdk.h>

void rgb_to_l(GdkPixbuf *image);
void vflip(GdkPixbuf *image);
void hflip(GdkPixbuf *image);
void l_quantize(GdkPixbuf *image, int q);

void negative(GdkPixbuf *image);
void brightness(GdkPixbuf *image, int b);
void contrast(GdkPixbuf *image, double c);
void zoom_out(GdkPixbuf *image, int sx, int sy);
void zoom_in(GdkPixbuf *image);
void rotate_right90(GdkPixbuf *image);
void rotate_left90(GdkPixbuf *image);
void convolute(GdkPixbuf *image, double **kernel);


#ifdef IMGPROC_IMPLEMENTATION
#include <stdlib.h>
#include <math.h>

#define map(i, j, k, x, n) (i*x*n + j*n + k)

void rgb_to_l(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
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

void vflip(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    char *tmp = malloc(x*n);
    for (int i = 0; i < (int) (y/2); i ++) {
        memcpy(tmp, data + i*x*n, x*n);
        memcpy(data + i*x*n, data + (y-i-1)*x*n, x*n);
        memcpy(data + (y-i-1)*x*n, tmp, x*n);
    }
    free(tmp);
}

void hflip(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
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

void l_quantize(GdkPixbuf *image, int q) {
    int n = gdk_pixbuf_get_n_channels(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
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

void negative(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    int Ri, Gi, Bi;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            Ri = map(i,j,0,x,n);
            Gi = map(i,j,1,x,n);
            Bi = map(i,j,2,x,n);
            data[Ri] = 255 - data[Ri];
            data[Gi] = 255 - data[Gi];
            data[Bi] = 255 - data[Bi];
        }
    }

}

void brightness(GdkPixbuf *image, int b) {
    int n = gdk_pixbuf_get_n_channels(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    int Ri, Gi, Bi, tmp;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            Ri = map(i,j,0,x,n);
            Gi = map(i,j,1,x,n);
            Bi = map(i,j,2,x,n);

            tmp = data[Ri] + b;
            tmp = (tmp < 0)? 0 : (tmp > 255)? 255 : tmp;
            data[Ri] = tmp;

            tmp = data[Gi] + b;
            tmp = (tmp < 0)? 0 : (tmp > 255)? 255 : tmp;
            data[Gi] = tmp;

            tmp = data[Bi] + b;
            tmp = (tmp < 0)? 0 : (tmp > 255)? 255 : tmp;
            data[Bi] = tmp;
        }
    }

}

void contrast(GdkPixbuf *image, double c) {
    int n = gdk_pixbuf_get_n_channels(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    int Ri, Gi, Bi;
    double tmp;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            Ri = map(i,j,0,x,n);
            Gi = map(i,j,1,x,n);
            Bi = map(i,j,2,x,n);

            tmp = data[Ri]*c;
            tmp = (tmp > 255)? 255 : tmp;
            data[Ri] = (int) tmp;

            tmp = data[Gi]*c;
            tmp = (tmp > 255)? 255 : tmp;
            data[Gi] = (int) tmp;

            tmp = data[Bi]*c;
            tmp = (tmp > 255)? 255 : tmp;
            data[Bi] = (int) tmp;
        }
    }

}

#endif

#endif
