#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "0Image.h"

Image readImage(char fname[]);
void Merge(       Image &im1, Image &im2);
void logicAnd(    Image &im1, Image &im2);
void logicNand(   Image &im1, Image &im2);
void logicOr(     Image &im1, Image &im2);
void logicXor(    Image &im1, Image &im2);
void Addition(    Image &im1, Image &im2);
void Subtraction( Image &im1, Image &im2);

void displayRgb(            Image &im);
void displayGray(           Image &im);
void linearContrast(        Image &im);
void histogramEqualization( Image &im);
int  calculateContrast(     Image &im);
void Luminance(             Image &im);
void gaussFilter(           Image &im);
void smoothingFilter(       Image &im);
void Sharpen(               Image &im);
void edgeDetect(            Image &im);
void robertFilter(          Image &im);
void prewittFilter(         Image &im);
void sobelFilter(           Image &im);
void laplacienConvo(        Image &im);
void Erosion(               Image &im);

void Hflip(                 Image &im);
void Vflip(                 Image &im);
void Rrotate(               Image &im);
void Lrotate(               Image &im);

void brightNess(                Image &im, int level);
void Filtering(                 Image &im, double r,   double g, double b);
void linearContrastSaturation(  Image &im, int sMin,   int sMax);
void Scaling(                   Image &im, double xscale, double yscale);
#endif