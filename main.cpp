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

double getCosAlKashi (const Point &A, const Point &B, const Point &C) {
    double CA = cv::norm(Mat(A), Mat(C));
    double CB = cv::norm(Mat(B), Mat(C));
    double AB = cv::norm(Mat(B), Mat(A));

    return (pow(CA, 2) + pow(CB, 2) - pow(AB, 2))/(2*CA*CB);
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
        vector<Point> curves;
        vector<Vec4i> hierarchy;
        findContours( dilation_dst, contours, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

        // Draw contours
        dst_shape = Mat::zeros( dilation_dst.size(), CV_8UC3 );
        for( size_t i = 0; i< contours.size(); i++ ) {

            // Approximation Douglas-Peucker
            double perimeter = arcLength(contours[i], true);
            approxPolyDP(contours[i], curves, 0.02 * perimeter, true);

            if (contourArea(curves) > 5000 && isContourConvex(curves)) {
                if (curves.size() == 3) {
                    // if we have 3 corner we have a triangle
                    cv::putText(frame, "TRIANGLE", curves[0], cv::FONT_HERSHEY_PLAIN, 2, Scalar(255, 0, 0, 255), 2);
                    drawContours(frame, contours, (int)i, Scalar(255, 0, 0, 255), 4, 8, hierarchy, 0, Point());
                } else if (curves.size() == 4) {
                    // If 4 angle are right-angled so we have a rectangle
                    double cos1 = getCosAlKashi(curves[2], curves[0], curves[1]);
                    double cos2 = getCosAlKashi(curves[3], curves[1], curves[2]);
                    double cos3 = getCosAlKashi(curves[0], curves[2], curves[3]);
                    double cos4 = getCosAlKashi(curves[1], curves[3], curves[2]);
                    cout << cos3 << endl;
                    if (cos1 < 0.1 && cos1 > -0.1 && cos2 < 0.1 && cos2 > -0.1 &&
                            cos3 < 0.1 && cos3 > -0.1 && cos4 < 0.1 && cos4 > -0.1) {
                        cv::putText(frame, "CARRE", curves[0], cv::FONT_HERSHEY_PLAIN, 2, Scalar(0, 255, 0, 255), 2);
                        drawContours(frame, contours, (int) i, Scalar(0, 255, 0, 255), 4, 8, hierarchy, 0, Point());
                    }
                } else {
                    // If we have the area close equal from the surface so we have a circle
                    Point2f center;
                    float radius;
                    minEnclosingCircle(curves, center, radius);
                    double area = contourArea(curves);
                    if (0.95 * CV_PI * pow(radius, 2) <= area <= 1.05 * CV_PI * pow(radius, 2)) {
                        putText(frame, "CERCLE", center, FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255, 255), 2);
                        drawContours(frame, contours, (int)i, Scalar(0, 0, 255, 255), 4, 8, hierarchy, 0, Point());
                    }
                }
            }
        }

        // Display the frame
        imshow( window_name, frame );

        // Press escape to quit
        if( waitKey(40) == 27 ) break;

        // Update the position of trackbar
        setTrackbarPos( trackbar_name, window_name, slider_pos );
        slider_pos++;

        // Get the actual frame
        int nextFrame = static_cast<int>(capture.get(CAP_PROP_POS_FRAMES));

        // If the frame is the last one, put back to 0
        if (nextFrame == count_frames) {
            nextFrame = 0;
            setTrackbarPos(trackbar_name, window_name, nextFrame);
            cout << "Pour redémarrer appuyez sur n'importe quelle touche (à part espace)" << endl;
            waitKey(0);
        }

    }

    // Reset to initial trackbar position
    slider_pos = 0;

    // Destroy the window
    destroyWindow( window_name );

    return 0;
}