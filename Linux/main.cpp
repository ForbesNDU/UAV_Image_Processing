////////// PROBABILISTIC COLOR TRACKER //////////
// Arguments: <gt_image> <gt_rect_file> <desired_fps>
// Author: Peter Forbes
// Date: 11/20/2015

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "img_proc.h"
#include "web_utils.h"

////////// MAIN //////////

int main(int argc, char **argv) {
	
	// Prepare log file and check argument count
	FILE* log_file = fopen("track_results.log","a");
	if(argc != 4) {
		fprintf(log_file, "Incorrect number of arguments.\n");
		return 1;
	}

	int desired_fps = atoi(argv[3]);
	if(desired_fps > 60 || desired_fps < 1) {
		fprintf(log_file, "Invalid FPS: please select a value in the range [1-60].\n");
		return 1;
	}

	////////// GROUND TRUTH SETUP AND PROCESSING //////////

	// Open and extract bounding rect info from gt file
	char buffer[100];
	memset(buffer, 0, sizeof(buffer));
	int gt_rect[4];

	FILE* gt_file = fopen(argv[2], "r");
	fgets(buffer, 100, gt_file);

	char* token = strtok(buffer, ",");
	gt_rect[0] = atoi(token);

	int i = 1;
	while(i < 4) {
		token = strtok(NULL, ",");
		gt_rect[i] = atoi(token);
		i++;
	}

	fclose(gt_file);

	// Load image and compress to a reasonable size
	IplImage* gt = cvLoadImage(argv[1]);

	IplImage* gt_resized = cvCreateImage(cvSize(320, 240), gt->depth, gt->nChannels); //1280,720
	cvResize(gt, gt_resized, CV_INTER_NN);

	// Show bounding rect
	CvPoint corner1 = cvPoint(gt_rect[0], gt_rect[1]);
	CvPoint corner2 = cvPoint(gt_rect[0] + gt_rect[2], gt_rect[1] + gt_rect[3]);
	CvScalar rect_color = CV_RGB(255,0,0);

	cvRectangle(gt_resized, corner1, corner2, rect_color, 2);

	cvNamedWindow( "Ground Truth Reference", CV_WINDOW_AUTOSIZE );
	cvShowImage( "Ground Truth Reference", gt_resized );

	// Set ROI for ground truth
	CvRect quarter = cvRect(gt_rect[0], gt_rect[1], gt_rect[2], gt_rect[3]);
	cvSetImageROI(gt_resized, quarter);

	////////// PREPARE GOPRO FOR VIDEO CAPTURE //////////

	// Power on
	bool error = ping_url("http://10.5.5.9/bacpac/PW?t=goprohero&p=%01");
	if(error) {
		return 1;
	}
	sleep(5); //give time to boot up

	//Clear memory
	error = ping_url("http://10.5.5.9/camera/DA?t=goprohero");
	if(error) {
		return 1;
	}
	sleep(5); //give time to delete files

	// Set to video mode
	error = ping_url("http://10.5.5.9/camera/CM?t=goprohero&p=%00");
	if(error) {
		return 1;
	}
	sleep(1);

	// Set video resolution to 720p, 30FPS
	error = ping_url("http://10.5.5.9/camera/VR?t=goprohero&p=%00");
	if(error) {
		return 1;
	}
	sleep(1);

	////////// PREPARE TIMING & VIDEO RESOURCES //////////

	// Prepare timing instrumentation (for FPS control)
	int frame_time = 1000 / desired_fps;

	// Play video
	cvNamedWindow( "MOV Window", CV_WINDOW_AUTOSIZE );
	CvCapture* track_video = cvCreateFileCapture( "tags.mov" );
	IplImage* current_frame;

	// Record annotated video
	CvSize write_size = cvSize(
       		(int)cvGetCaptureProperty( track_video, CV_CAP_PROP_FRAME_WIDTH),
       		(int)cvGetCaptureProperty( track_video, CV_CAP_PROP_FRAME_HEIGHT)
    	);	
	CvVideoWriter *writer = cvCreateVideoWriter( "output.avi", CV_FOURCC('M','J','P','G'), 20, write_size, 1);

	// Start timer
	struct timeval start, current;
	gettimeofday(&start, 0);

	////////// MAIN PROCESSING LOOP //////////

	bool to_search = true;
	bool next = true;
	CvRect est = quarter;

	while(1) {

		// Read in current frame
		current_frame = cvQueryFrame(track_video);
		if(current_frame == NULL) {
			break;
		}

		if(to_search == false) {
			est = process_frame(gt_resized, current_frame, quarter, &next, log_file);
			rect_color = CV_RGB(0,255,0);
		} else {
			est = search(gt_resized, current_frame, quarter, &next, log_file);
			rect_color = CV_RGB(255,0,0);
		}

		fprintf(log_file, "Coordinates: %d , %d\t\t", est.x, est.y);
		if(to_search) {
			fprintf(log_file, "Recommended Action: Search\n");
		} else {
			// X direction flight planning
			if(est.x < ((current_frame->width / 2) - 10)) {
				fprintf(log_file, "Recommended Action: Move Right , ");
			}
			else if(est.x > ((current_frame->width / 2) + 10)) {
				fprintf(log_file, "Recommended Action: Move Left, ");
			}
			else {
				fprintf(log_file, "Recommended Action: Hover, ");
			}
			
			// Y direction flight planning
			if(est.y < ((current_frame->height / 2) - 10)) {
				fprintf(log_file, "Move Backwards\n");
			}
			else if(est.y > ((current_frame->width / 2) + 10)) {
				fprintf(log_file, "Move Forwards\n");
			}
			else {
				fprintf(log_file, "Hover\n");
			}
		}
		to_search = next;

		// Swap frames
		quarter = est;

		CvPoint corner1 = cvPoint(est.x, est.y);
		CvPoint corner2 = cvPoint(est.x + est.width, est.y + est.height);

		cvRectangle(current_frame, corner1, corner2, rect_color, 2);

		// Display frame
		cvShowImage( "MOV Window", current_frame );
		cvWriteFrame( writer, current_frame );

		// FPS Control

		gettimeofday(&current, 0);
		

		int elapsed_time = ((current.tv_sec - start.tv_sec) * 1000000) + (current.tv_usec - start.tv_usec);
		int wait_time = frame_time - (elapsed_time / 1000);

		if(wait_time < 0) {
			continue;
		}

		char ext_key = cvWaitKey(wait_time);
		if(ext_key == 27) {
			break;
		}

	}

	////////// CLEAN-UP //////////

	cvReleaseCapture( &track_video );
	cvReleaseVideoWriter( &writer );
	cvDestroyWindow( "MOV Window" );

	cvReleaseImage( &gt );
	cvReleaseImage( &gt_resized );

	cvDestroyWindow( "Ground Truth Reference" );

	fclose(log_file);

	return 0;

}

