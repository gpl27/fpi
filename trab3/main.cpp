#include <opencv2/opencv.hpp>
using namespace cv;

typedef enum {
    EXIT,
    NEUTRAL, // r
    BLUR, // g
    EDGES, // e
    SOBEL, // s
    BRIGHT, // b
    CONTRAST, // c
    NEGATIVE, // n
    GRAY, // l
    RESIZE, // z
    ROTATE, // r
    VFLIP, // v
    HFLIP // h
} Adjustment;

typedef struct {
    Adjustment currAdj;

} State;

void getAdjustment(Adjustment *adj) {
    int keyPressed = pollKey();
    if (keyPressed > 0) {
        switch (keyPressed) {
            case 27: // ESC
                std::cout << "Exit" << std::endl;
                *adj = EXIT;
                break;
            case 82: // R
            case 114: // r
                std::cout << "Reset" << std::endl;
                *adj = NEUTRAL;
                break;
            case 71: // G
            case 103: // g
                std::cout << "Blurring" << std::endl;
                *adj = BLUR;
                break;
            case 69: // E
            case 101: // e
                std::cout << "Edges" << std::endl;
                *adj = EDGES;
                break;
            case 83: // S
            case 115: // s
                std::cout << "Sobel" << std::endl;
                *adj = SOBEL;
                break;
            case 66: // B
            case 98: // b
                std::cout << "Brightness" << std::endl;
                *adj = BRIGHT;
                break;
            case 67: // C
            case 99: // c
                std::cout << "Contrast" << std::endl;
                *adj = CONTRAST;
                break;
            case 78: // N
            case 110: // n
                std::cout << "Negative" << std::endl;
                *adj = NEGATIVE;
                break;
            case 76: // L
            case 108: // l
                std::cout << "Grayscale" << std::endl;
                *adj = GRAY;
                break;
            case 90: // Z
            case 122: // z
                std::cout << "Resize" << std::endl;
                *adj = RESIZE;
                break;
            default:
                break;
        }
    }
}


int main(int argc, char **argv) {
    const int trackbarSliderMax = 30;
    int tbSlider = 0;
    int camera = 0;
    Adjustment currAdj = NEUTRAL;
    int sztmp;
    Size ksize;
    int depth;
    Mat cpy;
    // Create Window
    namedWindow("Camera", WINDOW_KEEPRATIO);
    // resizeWindow("Camera", Size(0, 0));
    // Create Trackbar
    createTrackbar("Kernel Size", "Camera", &tbSlider, trackbarSliderMax, NULL); // Not sure if I need a callback
    VideoCapture cap;
    if (!cap.open(camera))
        return 0;
    while (currAdj != EXIT) {
        // Get Frame
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break; // end of video stream

        // Get adjustment change
        getAdjustment(&currAdj);

        // Apply adjustment
        switch (currAdj) {
            case NEUTRAL:
                break;
            case BLUR:
                cpy = frame.clone();
                sztmp = (tbSlider % 2)? tbSlider : tbSlider + 1;
                ksize = Size(sztmp, sztmp);
                GaussianBlur(cpy, frame, ksize, 0);
                break;
            case EDGES:
                cpy = frame.clone();
                Canny(cpy, frame, tbSlider, tbSlider*3);
                break;
            case SOBEL:
                cpy = frame.clone();
                depth = frame.type() & CV_MAT_DEPTH_MASK;
                sztmp = (tbSlider % 2)? tbSlider : tbSlider + 1;
                Sobel(cpy, frame, depth, 1, 0, sztmp);
                break;
            case BRIGHT:
                cpy = frame.clone();
                depth = frame.type() & CV_MAT_DEPTH_MASK;
                cpy.convertTo(frame, depth, 1.0, tbSlider);
                break;
            case CONTRAST:
                cpy = frame.clone();
                depth = frame.type() & CV_MAT_DEPTH_MASK;
                cpy.convertTo(frame, depth, tbSlider, 0);
                break;
            case NEGATIVE:
                cpy = frame.clone();
                depth = frame.type() & CV_MAT_DEPTH_MASK;
                cpy.convertTo(frame, depth, -1, 255);
                break;
            case GRAY:
                cvtColor(frame, frame, COLOR_RGB2GRAY);
                break;
            case RESIZE:
                cpy = frame.clone();
                resize(cpy, frame, Size(cpy.cols/2, cpy.rows/2));
                break;
            case ROTATE:
                break;
            case VFLIP:
                break;
            case HFLIP:
                break;
        }

        // Display frame
        imshow("Camera", frame);
    }
    cap.release(); // release the VideoCapture object
    return 0;
}