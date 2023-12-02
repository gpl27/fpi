#include <opencv2/opencv.hpp>
#include <list>
using namespace cv;

typedef enum {
    EXIT,
    NEUTRAL,
    RECORD,
    RESET, // r
    BLUR, // g
    EDGES, // e
    SOBEL, // s
    BRIGHT, // b
    CONTRAST, // c
    NEGATIVE, // n
    GRAY, // l
    RESIZE, // z
    ROTATER, // =
    ROTATEL, // -
    VFLIP, // v
    HFLIP // h
} Filter;

typedef struct {
    Filter filter;
    int sliderValue;
} Adjustment;

void printHelp() {
    std::cout << "Help Message\n"
    << "Press the keys to apply the respective filters. "
    << "Filters stack, you can use the slider to alter "
    << "the topmost filter in the stack."
    << "COMMANDS\n"
    << "R - RESET - Clears the stack of filters\n"
    << "G - Gaussian Blur\n"
    << "E - Edge Detection\n"
    << "S - Sobel Hx Filter\n"
    << "B - Brightness Adjustment\n"
    << "C - Contrast Adjustment\n"
    << "N - Negative\n"
    << "L - Luminance or Grayscale\n"
    << "Z - Resize Image by half\n"
    << "= - Rotate Right\n"
    << "- - Rotate Left\n"
    << "V - Vertical Flip\n"
    << "H - Horizontal Flip\n"
    << "M - Record video (Press to start/end)" << std::endl;
}

void getAdjustment(Filter *adj) {
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
                *adj = RESET;
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
            case 61: // =
                std::cout << "Rotate R" << std::endl;
                *adj = ROTATER;
                break;
            case 45: // -
                std::cout << "Rotate L" << std::endl;
                *adj = ROTATEL;
                break;
            case 86: // V
            case 118: // v
                std::cout << "VFlip" << std::endl;
                *adj = VFLIP;
                break;
            case 72: // H
            case 104: // h
                std::cout << "HFlip" << std::endl;
                *adj = HFLIP;
                break;
            case 77: // M
            case 109: // m
                *adj = RECORD;
                break;
            default:
                *adj = NEUTRAL;
                break;
        }
    } else {
        *adj = NEUTRAL;
    }
}

Mat applyAdjustment(Adjustment adj, Mat frame) {
    int tbSlider = adj.sliderValue;
    int sztmp;
    Size ksize;
    int depth;
    Mat cpy;
    switch (adj.filter) {
        case RESET:
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
            Sobel(cpy, frame, depth, 1, 0, sztmp, 1, 127);
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
            if (frame.channels() > 1)
                cvtColor(frame, frame, COLOR_RGB2GRAY);
            break;
        case RESIZE:
            cpy = Mat();
            resize(frame, cpy, Size(), 0.5, 0.5);
            frame = cpy;
            break;
        case ROTATER:
            transpose(frame, frame);
            flip(frame, frame, 1);
            break;
        case ROTATEL:
            transpose(frame, frame);
            flip(frame, frame, 0);
            break;
        case VFLIP:
            flip(frame, frame, 0);
            break;
        case HFLIP:
            flip(frame, frame, 1);
            break;
    }
    return frame;

}

void onTrackbarChange(int value, void *slider) {
    *((int *)slider) = value;
}

int main(int argc, char **argv) {
    int fourcc, fps;
    Size frameSize;
    VideoWriter writer;
    const int trackbarSliderMax = 30;
    int tbSlider = 0;
    int camera = 0;
    bool isRecording = false;
    Adjustment currAdj = {NEUTRAL, 0};
    Filter getFilter = NEUTRAL;
    std::list<Adjustment> currAdjs;
    // Create Window
    namedWindow("Camera", WINDOW_AUTOSIZE);
    // Create Trackbar
    createTrackbar("Kernel Size", "Camera", NULL, trackbarSliderMax, onTrackbarChange, &tbSlider); 
    VideoCapture cap;
    printHelp();
    if (!cap.open(camera))
        return 0;
    fourcc = static_cast<int>(cap.get(CAP_PROP_FOURCC)); 
    fps = cap.get(CAP_PROP_FPS);
    while (getFilter != EXIT) {
        // Get Frame
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break; // end of video stream

        // Get adjustment change
        currAdj.sliderValue = tbSlider;
        getAdjustment(&getFilter);
        if (getFilter == RESET) {
            currAdj.filter = NEUTRAL;
            currAdjs.clear();
        } else if (getFilter == RECORD) {
            if (isRecording) {
                std::cout << "Ended Recording." << std::endl;
                writer.release();
                isRecording = false;
            } else {
                // Get the current time and date
                auto now = std::chrono::system_clock::now();
                std::time_t time = std::chrono::system_clock::to_time_t(now);
                std::tm localTime = *std::localtime(&time);

                // Convert to a string with a specific format
                std::stringstream ss;
                ss << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S");  // Format: YYYY-MM-DD_HH-MM-SS
                std::string filename = ss.str() + ".avi";
                writer.open(filename, fourcc, fps, frameSize, true);
                isRecording = true;
                std::cout << "Started Recording " << filename << std::endl;
            }
        } else if (getFilter != NEUTRAL) {
            if (!(isRecording && (getFilter == RESIZE || getFilter == ROTATER || getFilter == ROTATEL))) {
                currAdjs.push_back(currAdj);
                currAdj.filter = getFilter;
            }
        }

        // Apply adjustments
        for (Adjustment adj : currAdjs) {
            frame = applyAdjustment(adj, frame);
        }
        frame = applyAdjustment(currAdj, frame);
                frameSize = Size(frame.cols, frame.rows);

        // Display frame
        imshow("Camera", frame);

        // Record frame
        if (isRecording) {
            if (frame.channels() != 3)
                cvtColor(frame, frame, cv::COLOR_GRAY2BGR);  // Adjust the conversion based on your needs
            writer.write(frame);
        }

    }
    cap.release(); // release the VideoCapture object
    if (isRecording)
        writer.release();
    destroyAllWindows();
    return 0;
}