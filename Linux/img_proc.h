#include "opencv/cv.h"

#ifndef IMG_PROC_H
#define IMG_PROC_H

CvRect process_frame(IplImage*, IplImage*, CvRect, bool*, FILE*);
CvRect search(IplImage*, IplImage*, CvRect, bool*, FILE*);
void set_differences(int*, int*, int*, CvRect* r0, CvRect *r1, CvRect *r2, int, CvRect);

#endif
