#include "img_proc.h"

// Given a frame, returns the predicted location based on the model if we have a good candidate
CvRect process_frame(IplImage* gt, IplImage* frame, CvRect last, bool* to_search, FILE *log_file) {
	
	int w = last.width;
	int h = last.height;
	IplImage *result = cvCreateImage(cvSize(w, h), gt->depth, gt->nChannels);

	int min_difference = INT_MAX;
	int min_difference2 = INT_MAX;
	int min_difference3 = INT_MAX;

	CvRect location = last;
	CvRect location1 = last;
	CvRect location2 = last;

	int max_diff = last.width * last.height * 255 * 3;
	
	// Sample 40 points
	for(float s = 0.1; s <= 0.5; s+=0.1) {
		for (int i = -1; i <= 1; i++) {
			for (int k = -1; k <= 1; k++) {
	
				// Set ROI for frame
				CvRect sample = cvRect((int)(last.x + s*w*i), (int)(last.y + s*h*k), w, h);
				if(sample.x < 0 || sample.y < 0 || (sample.x + w) >= frame->width || (sample.y + h) >= frame->height) {
					cvResetImageROI(frame);
					continue;
				}
				cvSetImageROI(frame, sample);
					
				// Compute difference
				cvAbsDiff(gt, frame, result);
				CvScalar diff_array = cvSum(result);
				int frame_difference = diff_array.val[0] + diff_array.val[1] + diff_array.val[2];
				
				if(frame_difference < min_difference3) {
					set_differences(&min_difference, &min_difference2, &min_difference3, &location, &location1, &location2, frame_difference, sample);
				}
	
				cvResetImageROI(frame);
			}
		}
	}	

	//fprintf(log_file, "%d / %d\n", min_difference, max_diff);
	cvResetImageROI(frame);

	double accuracy = (double)min_difference / (double)max_diff;
	
	// Switch to searching the entire image if a good match can't be found
	if(accuracy > 0.25) {
		//fprintf(log_file, "False\n");
		*to_search = true;
		return last;
	} else {
		CvRect prediction;
		prediction.x = location.x/6 + location1.x/6 + location2.x/6 + last.x/2;
		prediction.y = location.y/6 + location1.y/6 + location2.y/6 + last.y/2;
		prediction.width = w;
		prediction.height = h;
		return prediction;
	}

}

// Search for object matches when we don't have a good candidate
CvRect search(IplImage* gt, IplImage* frame, CvRect last, bool* to_search, FILE *log_file) {

	CvRect prediction;
	int w = last.width;
	int h = last.height;
	IplImage *result = cvCreateImage(cvSize(w, h), gt->depth, gt->nChannels);

	int min_difference = INT_MAX;
	int max_diff = last.width * last.height * 255 * 3;

	int height_step = frame->height / h; 
	int width_step = frame->width / w;

	for (float i = 0; i < width_step; i+=0.5) {
		for (float k = 0; k < height_step; k+=0.5) {
	
			// Set ROI for frame
			CvRect sample = cvRect((int)(i*w), (int)(k*h), w, h);
			if(sample.x < 0 || sample.y < 0 || (sample.x + w) >= frame->width || (sample.y + h) >= frame->height) {
				cvResetImageROI(frame);
				continue;
			}
			cvSetImageROI(frame, sample);
				
			// Compute difference
			cvAbsDiff(gt, frame, result);
			CvScalar diff_array = cvSum(result);
			int frame_difference = diff_array.val[0] + diff_array.val[1] + diff_array.val[2];
			
			if(frame_difference < min_difference) {
				min_difference = frame_difference;
				prediction = sample;
			}

			cvResetImageROI(frame);
		}
	}

	cvResetImageROI(frame);

	double accuracy = (double)min_difference / (double)max_diff;
	//fprintf(log_file, "%d / %d\n", min_difference, max_diff);

	if(accuracy < 0.25) {
		//fprintf(log_file, "True\n");
		*to_search = false;
		return prediction;
	} else {
		return last;
	}

}

// Orders most relevant samples
void set_differences(int* d0, int* d1, int* d2, CvRect* r0, CvRect *r1, CvRect *r2, int new_value, CvRect sample) {
	
	if(new_value < *d0) {
		*d2 = *d1;
		*d1 = *d0;
		*d0 = new_value;
		*r2 = *r1;
		*r1 = *r0;
		*r0 = sample;
	}
	else if(new_value > *d0 && new_value < *d1) {
		*d2 = *d1;
		*d1 = new_value;
		*r2 = *r1;
		*r1 = sample;
	}
	else if(new_value > *d1 && new_value < *d2) {
		*d2 = new_value;
		*r2 = sample;
	}

}
