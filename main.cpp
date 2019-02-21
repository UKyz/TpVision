#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int slider_pos = 0;
VideoCapture capture;

void onTrackbarSlide(int, void*)
{
    capture.set(CAP_PROP_POS_FRAMES, slider_pos);
}

int main() {

    // Declare local variables
    const char *window_name = "Video Capture";
    const char *trackbar_name = "Slider";
    slider_pos = 0;

    Mat frame, frameGray, dst2, dilation_dst, dst_shape, shape, dst_res;

    // Open the video
    capture.open( "images/video.MOV" );

    // Check if the video is valid
    if( !capture.isOpened() )
    {
        cout << "ERROR: Could not initialize capturing." << endl;
        return -1;
    }

    // Create a window
    namedWindow( window_name, WINDOW_AUTOSIZE );

    // Get the number of frames of the video
    int count_frames = (int)capture.get( CAP_PROP_FRAME_COUNT );

    // Create the trackbar
    if( count_frames )
        createTrackbar( trackbar_name, window_name, &slider_pos, count_frames, onTrackbarSlide);

    // Infinite loop
    while( true )
    {
        // Get the frame from video
        capture >> frame;

        // Check if the frame is valid
        if( frame.empty() )	break;

        // Convert the image to gray
        cvtColor( frame, frameGray, COLOR_RGB2GRAY );

        // Apply Canny
        int lowThreshold = 80;
        int highThreshold = 160;
        int kernelSize = 3;

        Canny( frameGray, dst2, lowThreshold, highThreshold, kernelSize);

        // Apply the dilation
        int dilation_size = 3;
        int dilation_type = MORPH_ELLIPSE;

        Mat dilatation_element = getStructuringElement( dilation_type,
                                                        Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                                        Point( dilation_size, dilation_size ) );

        dilate( dst2, dilation_dst, dilatation_element );

        // Find contours
        RNG rng(12345);
        vector<vector<Point>> contours;
        vector<Point> curve;
        vector<Vec4i> hierarchy;
        findContours( dilation_dst, contours, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

        // Draw contours
        dst_shape = Mat::zeros( dilation_dst.size(), CV_8UC3 );
        for( size_t i = 0; i< contours.size(); i++ ) {

            // Approximation Douglas-Hecker
            double perimeter = arcLength(contours[i], true);
            approxPolyDP(contours[i], curve, 0.02*perimeter, true);

            if (contourArea(curve) > 5000 && isContourConvex(curve)) {

                Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
                drawContours(dst_shape, contours, (int)i, color, 4, 8, hierarchy, 0, Point());

            }
        }

        // Display the frame
        imshow( window_name, dst_shape );

        // Press escape to quit
        if( waitKey(40) == 27 ) break;

        // Update the position of trackbar
        setTrackbarPos( trackbar_name, window_name, slider_pos );
        slider_pos++;
    }

    // Reset to initial trackbar position
    slider_pos = 0;

    // Destroy the window
    destroyWindow( window_name );

    return 0;
}