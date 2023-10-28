#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STBI_ONLY_JPEG

#define JPEG_QLTY 100

#include "imgproc.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Not enough args.\n");
        return 1;
    }
    char *filename = argv[1];
    printf("%s\n", filename);

    int x, y, n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);

    printf("Width: %d\nHeight: %d\nChannels: %d\n", x, y, n);

    rgb_to_l(data, x, y, n);
    // vflip(data, x, y, n);
    // hflip(data, x, y, n);

    stbi_write_jpg("teste.jpg", x, y, n, data, JPEG_QLTY);

    stbi_image_free(data);

    return 0;
}