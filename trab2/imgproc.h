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
GdkPixbuf *rotate_right90(GdkPixbuf *image);
GdkPixbuf *rotate_left90(GdkPixbuf *image);
GdkPixbuf *zoom_in(GdkPixbuf *image);
GdkPixbuf *zoom_out(GdkPixbuf *image, int sx, int sy);
void calculate_histogram(GdkPixbuf *image, unsigned int *hist); // NEEDS TESTING
void convolute(GdkPixbuf *image, double kernel[3][3]); // Could use more tests
GdkPixbuf *create_histogram_img(unsigned int *hist); // NEEDS TESTING


#ifdef IMGPROC_IMPLEMENTATION
#include <stdlib.h>
#include <math.h>

#define map(i, j, k, rs, n) (i*rs + j*n + k)

void rgb_to_l(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    int Ri, Gi, Bi;
    unsigned char L;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            Ri = map(i,j,0,rs,n);
            Gi = map(i,j,1,rs,n);
            Bi = map(i,j,2,rs,n);
            L = (unsigned char) (0.299*data[Ri] + 0.587*data[Gi] + 0.114*data[Bi]);
            memset(&data[Ri], L, n);
        }
    }
}

void vflip(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    char *tmp = malloc(rs);
    int slast_row = x * ((n * 8 + 7) / 8);
    // First swap top and bottom pixel rows because
    // of gdk pixbuf properties (last row might not have
    // padding)
    memcpy(tmp, &data[map((y-1),0,0,rs,n)], slast_row);
    memcpy(&data[map((y-1),0,0,rs,n)], data, slast_row);
    memcpy(data, tmp, slast_row);
    for (int i = 1; i < (int) (y/2) - 1; i++) {
        memcpy(tmp, &data[map(i,0,0,rs,n)], rs);
        memcpy(&data[map(i,0,0,rs,n)], &data[map((y-i-1),0,0,rs,n)], rs);
        memcpy(&data[map((y-i-1),0,0,rs,n)], tmp, rs);
    }
    free(tmp);
}

void hflip(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    char *tmp = malloc(n*sizeof(unsigned char));
    for (int j = 0; j < (int) (x/2) ; j++) {
        for (int i = 0; i < y; i++) {
            memcpy(tmp, &data[map(i,j,0,rs,n)], n);
            memcpy(&data[map(i,j,0,rs,n)], &data[map(i,(x-j-1),0,rs,n)], n);
            memcpy(&data[map(i,(x-j-1),0,rs,n)], tmp, n);
        }
    }
    free(tmp);
}

void l_quantize(GdkPixbuf *image, int q) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    // Find t1 and t2 (min and max)
    int t1 = data[0];
    int t2 = data[0];
    int idx;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            idx = map(i,j,0,rs,n);
            t1 = (data[idx] < t1)? data[idx] : t1; // min
            t2 = (data[idx] > t2)? data[idx] : t2; // max
        }
    }
    int int_size = t2 - t1 + 1;
    if (q >= int_size)
        return; // No quantization is necessary
    
    // Bin and quantize
    float bin_size = (float) int_size / (float) q;
    int L, bin_id;
    float Li, Lj;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            idx = map(i,j,0,rs,n);
            bin_id = (data[idx] - t1) / bin_size;
            Li = t1 + bin_id * bin_size;
            Lj = t1 + (bin_id+1) * bin_size;
            L = round((double)(Li + Lj)/2);
            memset(&data[idx], L, n);
        }
    }
}

void negative(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    int Ri, Gi, Bi;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            Ri = map(i,j,0,rs,n);
            Gi = map(i,j,1,rs,n);
            Bi = map(i,j,2,rs,n);
            data[Ri] = 255 - data[Ri];
            data[Gi] = 255 - data[Gi];
            data[Bi] = 255 - data[Bi];
        }
    }

}

void brightness(GdkPixbuf *image, int b) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    int Ri, Gi, Bi, tmp;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            Ri = map(i,j,0,rs,n);
            Gi = map(i,j,1,rs,n);
            Bi = map(i,j,2,rs,n);

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
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    int Ri, Gi, Bi;
    double tmp;
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            Ri = map(i,j,0,rs,n);
            Gi = map(i,j,1,rs,n);
            Bi = map(i,j,2,rs,n);

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

GdkPixbuf *rotate_right90(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    GdkPixbuf *rimage = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(image), FALSE, 8, y, x);
    unsigned char *rdata = gdk_pixbuf_get_pixels(rimage);

    int k = 0;
    for (int i = 0; i < x; i++) {
        for (int j = y - 1; j >= 0; j--) {
            memcpy(&rdata[k], &data[map(j,i,0,rs,n)], n);
            k += n;
        }
    }

    g_object_unref(image);
    return rimage;
}

GdkPixbuf *rotate_left90(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    GdkPixbuf *rimage = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(image), FALSE, 8, y, x);
    unsigned char *rdata = gdk_pixbuf_get_pixels(rimage);

    int k = 0;
    for (int i = x - 1; i >= 0; i--) {
        for (int j = 0; j < y; j++) {
            memcpy(&rdata[k], &data[map(j,i,0,rs,n)], n);
            k += n;
        }
    }

    g_object_unref(image);
    return rimage;
}

GdkPixbuf *zoom_in(GdkPixbuf *image) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    int xn = 2*x - 1;
    int yn = 2*y - 1;
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    GdkPixbuf *nimage = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(image), gdk_pixbuf_get_has_alpha(image), 8, xn, yn);
    unsigned char *ndata = gdk_pixbuf_get_pixels(nimage);
    int nrs = gdk_pixbuf_get_rowstride(nimage);

    // Transfer pixels
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            memcpy(&ndata[map(2*i,2*j,0,nrs,n)], &data[map(i,j,0,rs,n)], n);
        }
    }

    // Interpolate Rows
    for (int i = 0; i < yn; i += 2) {
        for (int j = 1; j < xn - 1; j += 2) {
            for (int k = 0; k < n; k++) {
                ndata[map(i,j,k,nrs,n)] = (unsigned char)((ndata[map(i,(j-1),k,nrs,n)] + ndata[map(i,(j+1),k,nrs,n)])/2);
            }
        }
    }
    // Interpolate Columns
    for (int j = 0; j < xn; j++) {
        for (int i = 1; i < yn - 1; i += 2) {
            for (int k = 0; k < n; k++) {
                ndata[map(i,j,k,nrs,n)] = (unsigned char)((ndata[map((i-1),j,k,nrs,n)] + ndata[map((i+1),j,k,nrs,n)])/2);
            }
        }
    }

    g_object_unref(image);
    return nimage;
}

GdkPixbuf *zoom_out(GdkPixbuf *image, int sx, int sy) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    int xn = ceil((double)x/(double)sx);
    int yn = ceil((double)y/(double)sy);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    GdkPixbuf *nimage = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(image), gdk_pixbuf_get_has_alpha(image), 8, xn, yn);
    unsigned char *ndata = gdk_pixbuf_get_pixels(nimage);
    int nrs = gdk_pixbuf_get_rowstride(nimage);


    int idx, idy;
    int sum = 0;
    int avg = 0;
    int npixels = sx*sy;
    // For every pixel and channel in the new image
    for (int i = 0; i < yn; i++) {
        for (int j = 0; j < xn; j++) {
            for (int k = 0; k < n; k++) {
                sum = 0;
                // Iterate through the sub sx by sy pixels in the original image
                for (int l = 0; l < sy; l++) {
                    for (int m = 0; m < sx; m++) {
                        idx = (j*sx) + m;
                        idx = (idx < x)? idx : x - 1;
                        idy = (i*sy) + l;
                        idy = (idy < y)? idy : y - 1;
                        sum += data[map(idy,idx,k,rs,n)];
                    }
                }
                avg = (int)sum/npixels;
                ndata[map(i,j,k,nrs,n)] = (unsigned char)avg;
            }
        }
    }

    g_object_unref(image);
    return nimage;
}

// Assumes hist is already allocated and that image is grayscale
void calculate_histogram(GdkPixbuf *image, unsigned int *hist) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);
    int sum = x*y;

    // Resets hist to zero
    memset(hist, 0, 256);

    // Pixel counting
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            hist[data[map(i,j,0,rs,n)]]++;
        }
    }

    // Normalizes histogram to 0-256
    for (int i = 0; i < 256; i++)
        hist[i] = (hist[i]*256)/sum;
}

// Assumes kernel is a 3x3 matrix
void convolute(GdkPixbuf *image, double kernel[3][3]) {
    int n = gdk_pixbuf_get_n_channels(image);
    int rs = gdk_pixbuf_get_rowstride(image);
    g_assert(gdk_pixbuf_get_bits_per_sample(image) == 8);
    int x = gdk_pixbuf_get_width(image);
    int y = gdk_pixbuf_get_height(image);
    unsigned char *data = gdk_pixbuf_get_pixels(image);

    // Create temporary copy of image
    GdkPixbuf *imagecpy = gdk_pixbuf_copy(image);
    unsigned char *datacpy = gdk_pixbuf_get_pixels(imagecpy);
    int nrs = gdk_pixbuf_get_rowstride(imagecpy);

    double sum;
    for (int i = 1; i < y-1; i++) {
        for (int j = 1; j < x-1; j++) {
            for (int k = 0; k < n; k++) {
                sum = 0;
                for (int l = 0; l < 3; l++) {
                    for (int m = 0; m < 3; m++) {
                        sum += datacpy[map((i-1+l),(j-1+m),k,nrs,n)]*kernel[2-l][2-m];
                    }
                }
                sum = (sum < 0)? 0 : sum;
                sum = (sum > 255)? 255 : sum;
                data[map(i,j,k,rs,n)] = (unsigned char) sum;
            }
        }
    }

    g_object_unref(imagecpy);
}

GdkPixbuf *create_histogram_img(unsigned int *hist) {
    int x = 256;
    int y = 256;
    int sum = 0;
    GdkPixbuf *histogram = gdk_pixbuf_new(NULL, FALSE, 8, x, y);
    // Gets number of pixels
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            // TODO
        }
    }



}

#endif

#endif
