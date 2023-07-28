#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include "0Image.h"
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
#include <vector>
#include "0functions.h"
#define OUT_PATH            "./sample.jpg"

void displayGray(   Image &img){
    Mat ImgG(img.rows, img.cols, CV_8UC1, Scalar(0,0,0));
    for(int i = 0; i < img.rows; i++){
        for(int j = 0; j < img.cols; j++){
            ImgG.at<uchar>(i,j) = img.getPixelVal(i, j);
        }
    }
    cout<<"cols: "<<img.cols<<"ncols: "<<img.rows<<endl;
    imwrite(OUT_PATH, ImgG);
    imshow("window name", ImgG);
    waitKey(0);
}
void displayRGB(    Image &img){
    Mat ImgG(img.rows, img.cols, CV_8UC3, Scalar(0,0,0));
    for(int i = 0; i < img.rows; i++){
        for(int j = 0; j < img.cols; j++){
            ImgG.at<Vec3b>(i,j).val[0] = (uchar)img.getPixelVal3(i,j,0);
            ImgG.at<Vec3b>(i,j).val[1] = (uchar)img.getPixelVal3(i,j,1);
            ImgG.at<Vec3b>(i,j).val[2] = (uchar)img.getPixelVal3(i,j,2);
        }
    }
    cout<<"cols: "<<img.cols<<"ncols: "<<img.rows<<endl;
    imwrite(OUT_PATH, ImgG);
    imshow("window name", ImgG);
    waitKey(0);    
}
void grayScale(     Image &img){
    if(img.depth == 3){
        int pixel = 0, val = 0;
        for(int i = 0; i < img.rows; i++){
            for(int j = 0; j < img.cols; j++){
                int val = (int)(
                  (0.3  * (double)img.getPixelVal3(i, j, 0))
                + (0.59 * (double)img.getPixelVal3(i, j, 1))
                + (0.11 * (double)img.getPixelVal3(i, j, 2))
                );
                img.setPixelVal(i, j, val);
            }
        }
    }  
}

Image readImage(    char fname[]){
    Mat image = imread(fname);
    if(image.empty()){
        cout<<"Image file not found"<<endl;
        cin.get();
    }
    Image img(image.rows, image.cols, image.channels());

    for(int i = 0; i < image.rows; i++){
        for(int j = 0; j < image.cols; j++){
            Vec3b intensity = image.at<Vec3b>(i,j);
            img.setPixelVal3(i, j, 0, intensity.val[0]);
            img.setPixelVal3(i, j, 1, intensity.val[1]);
            img.setPixelVal3(i, j, 2, intensity.val[2]);
        }
    }
    waitKey(0);
    grayScale(img);
    cout<<"number of channels: "<<image.channels()<<endl;
    return img;
}
Image thresholdVal( int threshold, Image &im) {
    int pixel = 0, val = 0;
    Image newImage(im.rows, im.cols, im.depth);
    for (int i = 0; i < im.rows; i++) {
        for (int j = 0; j < im.cols; j++) {
            pixel = im.getPixelVal(i, j);
            val = pixel >= threshold ? 255 : 0;
            newImage.setPixelVal(i, j, val);
        }
    }
 //   displayGray(im);
 //   displayGray(newImage);
    return newImage;
}
Image otsuBinarize( Image &im){
    float histogram[255] = {0};
    float sum = 0;
    for(int i = 0; i < 255; i++){
        histogram[i] = 0;
    }
    for(int i = 0; i < im.rows; i++){
        for(int j = 0; j < im.cols; j++){
            int pixel = im.getPixelVal(i,j);
            histogram[pixel]++;
        }
    }
    for(int i = 0; i < 255; i++){
        sum += histogram[i] * (float)i;
    }
    float sumB    = 0;
    float wB      = 0;
    float wF      = 0;
    float varMax  = 0;
    int threshold = 0;

    for(int t = 0; t < 255; t++){
        wB += histogram[t];
        if(wB == 0)continue;

        wF = (im.rows * im.cols) - wB;
        if(wF == 0)break;

        sumB += (float)(t * histogram[t]);
        float mB = sumB / wB;
        float mF = ((float)sum - sumB)/ wF;

        float varBetween = wB * wF * (mB - mF) * (mB - mF);

        if(varBetween > varMax){
            varMax = varBetween;
            threshold = t;
        }
    }
    cout << "otso threshold value :"<< threshold<<endl;
    return thresholdVal(threshold, im);
}
int max(            int a, int b){
    return ((a >= b) ? a : b);
}
int min(            int a, int b){
    return ((a <= b) ? a : b);
}

int maxPixel(               Image &im){
    int maxi = 0, pixel = 0;
    int val = 0;
    for (int i = 0; i < im.rows; i++){
        for (int j = 0; j < im.cols; j++){
            pixel = im.getPixelVal(i, j);
            maxi = max(pixel, maxi);
        }
    }
    return maxi;
}
int minPixel(               Image &im){
    int mini = im.depth;
    int pixel = 0;
    int val = 0;
    for (int i = 0; i < im.rows; i++){
        for (int j = 0; j < im.cols; j++){
            pixel = im.getPixelVal(i, j);
            mini = min(pixel, mini);
        }
    }
    return mini;
}
void linearContrast(        Image &im){
    Image newImage  = Image(im.rows, im.cols, im.depth);
    int mini        = minPixel(im);

    int pixel       = 0;
    int val         = 0;
    int contrast    = calculateContrast(im);
    for (int i = 0; i < im.rows; i++){
        for (int j = 0; j < im.cols; j++){
            pixel = im.getPixelVal(i, j);
            val = 255 * (pixel - mini) / contrast;
            newImage.setPixelVal(i, j, val);
        }
    }
}
void histogramEqualization( Image &im){
    int pixel = 0;
    int histogram[256]  = {0};
    int cdf[256]        = {0};
    int sk[256]         = {0};
    int   kk[256]       = {0};
    int size            = im.cols * im.rows;

    Image newImage  = Image(im.rows, im.cols, im.depth);

    for (int i = 0; i < im.rows; i++){
        for (int j = 0; j < im.cols; j++){
            int pixel = im.getPixelVal(i, j);
            histogram[pixel]++;
        }
    }
    cdf[0] = histogram[0];
    int cdfmin = histogram[0];
    for (int i = 1; i < 256; i++)
    {
        cdf[i] = histogram[i] + cdf[i - 1];
        cdfmin = histogram[i] == 0 ? cdfmin : min(histogram[i], cdfmin);
    }
    for (int i = 0; i < 256; i++)
    {
        sk[i] = (((cdf[i] - cdfmin) * (256 - 2))/(size - cdfmin)) + 1;
    }

    for (int i = 0; i < 256; i++)
    {
        cout<<i<<" | "<<histogram[i]<<" | "<<cdf[i]<<" | "<<sk[i]<<" | "<<cdfmin<<" | "<<endl;
    }
    for (int i = 0; i < im.rows; i++)
    {
        for (int j = 0; j < im.cols; j++)
        {
            pixel = im.getPixelVal(i, j);
            newImage.setPixelVal(i, j, sk[pixel]);
        }
    }
    displayGray(im);
    displayGray(newImage);
}
int calculateContrast(     Image &im){
    int contrast= 0;
    int mini    = minPixel(im);
    int maxi    = maxPixel(im);
    int val     = 0;
    contrast    = maxi - mini;
    cout<<"contrast: "<<contrast<<endl;
    return contrast;
}
void Luminance(             Image &im){
    double lum = 0, sum = 0;
    int total = im.cols * im.rows, pixel = 0;
    for (int i = 0; i < im.rows; i++){
        for (int j = 0; j < im.cols; j++){
            sum += im.getPixelVal(i, j);
        }
    }
    lum = sum / total;
    cout<<"Luminance: "<<lum<<endl;    
}

void Merge(                 Image &im1, Image &im2){
    if(!(im1.depth == im2.depth) || !(im1.rows == im2.rows) || !(im1.cols == im2.cols)){
        cout<<"images must have same depth, rows and colums"<<endl;
        exit(1);
    }
    int pixel = 0, val = 0;
    Image newIm1 = otsuBinarize(im1);
    Image newImage = Image(im1.rows, im1.cols, im1.depth);
    for (int i = 0; i < im1.rows; i++)
    {
        for (int j = 0; j < im1.cols; j++)
        {
            if (newIm1.getPixelVal(i, j) == 0) {
                newImage.setPixelVal3(i, j, 0, im1.getPixelVal3(i, j, 0));
                newImage.setPixelVal3(i, j, 1, im1.getPixelVal3(i, j, 1));
                newImage.setPixelVal3(i, j, 2, im1.getPixelVal3(i, j, 2));
            }
            else {
                newImage.setPixelVal3(i, j, 0, im2.getPixelVal3(i, j, 0));
                newImage.setPixelVal3(i, j, 1, im2.getPixelVal3(i, j, 1));
                newImage.setPixelVal3(i, j, 2, im2.getPixelVal3(i, j, 2));
            }
        }
    } 
    displayRGB(newImage);
}
void logicAnd(              Image &im1, Image &im2){
    if(!(im1.depth == im2.depth) || !(im1.rows == im2.rows) || !(im1.cols == im2.cols)){
        cout<<"images must have same depth, rows and colums"<<endl;
        exit(1);
    }
    int pixel = 0, val = 0;
    Image newIm1 = otsuBinarize(im1);
    Image newIm2 = otsuBinarize(im2);
    Image newImage = Image(im1.rows, im1.cols, im1.depth);
    for (int i = 0; i < im1.rows; i++)
    {
        for (int j = 0; j < im1.cols; j++)
        {
            pixel = newIm1.getPixelVal(i, j) && newIm2.getPixelVal(i, j);
            val = (pixel == 1) ? 255 : 0; //NB: 255 && 255=1, 255 && 0=0
            newImage.setPixelVal(i, j, val);
        }
    } 
    displayGray(newImage);
}
void logicNand(             Image &im1, Image &im2){
    if(!(im1.depth == im2.depth) || !(im1.rows == im2.rows) || !(im1.cols == im2.cols)){
        cout<<"images must have same depth, rows and colums"<<endl;
        exit(1);
    }
    int pixel = 0, val = 0;
    Image newIm1 = otsuBinarize(im1);
    Image newIm2 = otsuBinarize(im2);
    Image newImage = Image(im1.rows, im1.cols, im1.depth);
    for (int i = 0; i < im1.rows; i++)
    {
        for (int j = 0; j < im1.cols; j++)
        {
            pixel = !(newIm1.getPixelVal(i, j) && newIm2.getPixelVal(i, j));
            val = (pixel == 1) ? 255 : 0; //NB: 255 && 255=1, 255 && 0=0
            newImage.setPixelVal(i, j, val);
        }
    }  
    displayGray(im1);
    displayGray(im2);
    displayGray(newImage);    
}
void logicOr(               Image &im1, Image &im2){
    if(!(im1.depth == im2.depth) || !(im1.rows == im2.rows) || !(im1.cols == im2.cols)){
        cout<<"images must have same depth, rows and colums"<<endl;
        exit(1);
    }
    int pixel = 0, val = 0;
    Image newIm1 = otsuBinarize(im1);
    Image newIm2 = otsuBinarize(im2);
    Image newImage = Image(im1.rows, im1.cols, im1.depth);
    for (int i = 0; i < im1.rows; i++)
    {
        for (int j = 0; j < im1.cols; j++)
        {
            pixel = newIm1.getPixelVal(i, j) || newIm2.getPixelVal(i, j);
            val = (pixel == 1) ? 255 : 0; //NB: 255 && 255=1, 255 && 0=0
            newImage.setPixelVal(i, j, val);
        }
    }  
    displayGray(im1);
    displayGray(im2);
    displayGray(newImage);    
}
void logicXor(              Image &im1, Image &im2){
    if(!(im1.depth == im2.depth) || !(im1.rows == im2.rows) || !(im1.cols == im2.cols)){
        cout<<"images must have same depth, rows and colums"<<endl;
        exit(1);
    }
    int pixel = 0, val = 0;
    Image newIm1 = otsuBinarize(im1);
    Image newIm2 = otsuBinarize(im2);
    Image newImage = Image(im1.rows, im1.cols, im1.depth);
    for (int i = 0; i < im1.rows; i++)
    {
        for (int j = 0; j < im1.cols; j++)
        {
            pixel = (newIm1.getPixelVal(i, j) && !(newIm2.getPixelVal(i, j)))
             || (newIm2.getPixelVal(i, j) && !(newIm1.getPixelVal(i, j)));
            val = (pixel == 1) ? 255 : 0; //NB: 255 && 255=1, 255 && 0=0
            newImage.setPixelVal(i, j, val);
        }
    }  
    displayGray(im1);
    displayGray(im2);
    displayGray(newImage);    
}
void Addition(              Image &im1, Image &im2){
    if(!(im1.depth == im2.depth) || !(im1.rows == im2.rows) || !(im1.cols == im2.cols)){
        cout<<"images must have same depth, rows and colums"<<endl;
        exit(1);
    }
    int pixel = 0, val = 0;
    Image newIm1 = otsuBinarize(im1);
    Image newIm2 = otsuBinarize(im2);
    Image newImage = Image(im1.rows, im1.cols, im1.depth);
    for (int i = 0; i < im1.rows; i++)
    {
        for (int j = 0; j < im1.cols; j++)
        {
            pixel = newIm1.getPixelVal(i, j) + newIm2.getPixelVal(i, j);
            val = min(pixel, 255); //NB: 255 && 255=1, 255 && 0=0
            newImage.setPixelVal(i, j, val);
        }
    }  
    displayGray(im1);
    displayGray(im2);
    displayGray(newImage);    
}
void Subtraction(           Image &im1, Image &im2){
    if(!(im1.depth == im2.depth) || !(im1.rows == im2.rows) || !(im1.cols == im2.cols)){
        cout<<"images must have same depth, rows and colums"<<endl;
        exit(1);
    }
    int pixel = 0, val = 0;
    Image newIm1 = otsuBinarize(im1);
    Image newIm2 = otsuBinarize(im2);
    Image newImage = Image(im1.rows, im1.cols, im1.depth);
    for (int i = 0; i < im1.rows; i++)
    {
        for (int j = 0; j < im1.cols; j++)
        {
            pixel = newIm1.getPixelVal(i, j) - newIm2.getPixelVal(i, j);
            val = max(pixel, 0); //NB: 255 && 255=1, 255 && 0=0
            newImage.setPixelVal(i, j, val);
        }
    }  
    displayGray(im1);
    displayGray(im2);
    displayGray(newImage);     
}

Image convolution(Image &im, double kernel[3][3], int kSize, int norm){
    Image newImage = Image(im.rows, im.cols, im.depth);
    int kCenter = kSize / 2;
    double sum  = 0;

    for(int i = 0; i < im.rows; i++){
        for(int j = 0; j < im.cols; j++){
            for(int m = 0; m < kSize; m++){
                for(int n = 0; n < kSize; n++){
                    if((i + m - kCenter < 0) || (i + m - kCenter >= im.rows))
                        break;
                    if((j + n - kCenter < 0) || (j + n - kCenter >= im.cols))
                        continue;
                    
                    sum += im.getPixelVal(i + m - kCenter, j + n - kCenter) * kernel[m][n];
                }
            }
            double val = sum / norm;
            val = val < 0 ? 0 : (double)min((int)val, 255);
            newImage.setPixelVal(i, j, (int)val);
            sum = 0;
        }
    }
    return newImage;
}
Image convo2D(Image &im, double kernel[3][3], int kSize, int norm){
    double kern[3][3];
    for(int i = 0; i < kSize; i++){
        for(int j = 0; j < kSize; j++){
            kern[i][j] = kernel[kSize - i - 1][kSize - j - 1];
        }
    }
    return(convolution(im, kern, kSize, norm));
}
void gaussFilter(           Image &im){
    double gaussBlur[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}};
    Image final = convolution(im, gaussBlur, 3, 16); 
    displayGray(im);
    displayGray(final);    
}
void smoothingFilter(       Image &im){
    double kern[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}};
    Image final = convolution(im, kern, 3, 9);
    displayRGB(final);
    displayGray(final);     
}
void Sharpen(               Image &im){
    double kern[3][3] = {
        {1.0 / 4, 1.0 / 2, 1.0 / 4},
        {1.0 / 2, 1, 1.0 / 2},
        {1.0 / 4, 1.0 / 2, 1.0 / 4}};
    Image final = convolution(im, kern, 3, 4);
    displayGray(im);
    displayGray(final); 
}
void edgeDetect(            Image &im){
    double kern[3][3] = {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}};
    Image final = convolution(im, kern, 3, 1);  
    displayGray(im);
    displayGray(final);     
}
void robertFilter(          Image &im){
    double kern1[3][3] = {
        {-1, 0, 0},
        {0, -1, 0},
        {0, 0, 0}
    };
    double kern2[3][3] = {
        {0, 1, 0},
        {-1, 0, 0},
        {0, 0, 0}
    };
    Image temp = convo2D(im, kern1, 2, 1);
    Image final= convo2D(temp, kern2, 2, 1);    
    displayGray(im);
    displayGray(final);    
}
void prewittFilter(         Image &im){
    double kern1[3][3] = {
        {-1, -1, -1},
        {0, 0, 0},
        {1, 1, 1}};
    double kern2[3][3] = {
        {-1, 0, 1},
        {-1, 0, 1},
        {-1, 0, 1}};

    Image temp = convolution(im, kern1, 3, 1);
    Image final = convolution(temp, kern2, 3, 1);     
    displayRGB(final);
    displayGray(final);      
}
void sobelFilter(           Image &im){
    double kern1[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};
    double kern2[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};
    Image temp  = convolution(im, kern1, 3, 1);
    Image final = convolution(temp, kern2, 3, 1);
    displayRGB(final);
    displayGray(final);    
}
void laplacienConvo(        Image &im){
    double kern[3][3] = {
        {1, 1, 1},
        {1, -8, 1},
        {1, 1, 1}};
    Image final = convolution(im, kern, 3, 1);
    displayGray(im);
    displayGray(final);      
}
void Erosion(               Image &im){
    Image binImage = otsuBinarize(im);
    double kern[3][3] = {
         {0,0,0},
        {0,1,0},
        {0,0,0}};
    Image final = convolution(binImage, kern, 3, 3);
    displayGray(im);
    displayGray(final);  
    
}

void Hflip(                 Image &im){
    Image newImage = Image(im.rows, im.cols, im.depth);
    for (int i = 0; i < im.rows; i++)
    {
        for (int j = 0; j < im.cols; j++)
        {
            newImage.setPixelVal3(i, j, 0, im.getPixelVal3(i, im.cols-j-1, 0));
            newImage.setPixelVal3(i, j, 1, im.getPixelVal3(i, im.cols-j-1, 1));
            newImage.setPixelVal3(i, j, 2, im.getPixelVal3(i, im.cols-j-1, 2));
        }
    }
    displayRGB(im);    
    displayRGB(newImage);
}
void Vflip(                 Image &im){
    Image newImage = Image(im.rows, im.cols, im.depth);
    for (int i = 0; i < im.rows; i++)
    {
        for (int j = 0; j < im.cols; j++)
        {
            newImage.setPixelVal3(i, j, 0, im.getPixelVal3(im.rows-i-1, j, 0));
            newImage.setPixelVal3(i, j, 1, im.getPixelVal3(im.rows-i-1, j, 1));
            newImage.setPixelVal3(i, j, 2, im.getPixelVal3(im.rows-i-1, j, 2));
        }
    }
    displayRGB(im);
    displayRGB(newImage);
}
void Rrotate(               Image &im){
    Image newImage = Image(im.cols, im.rows, im.depth);
    for(int i = 0; i < im.rows; i++){
        for(int j = 0; j < im.cols; j++){
            newImage.setPixelVal3(im.cols-j-1, i, 0, im.getPixelVal3(i,j,0));
            newImage.setPixelVal3(im.cols-j-1, i, 1, im.getPixelVal3(i,j,1));
            newImage.setPixelVal3(im.cols-j-1, i, 2, im.getPixelVal3(i,j,2));
        }
    }
    displayRGB(im);
    displayRGB(newImage);
}
void Lrotate(               Image &im){
    Image newImage = Image(im.cols, im.rows, im.depth);
    for(int i = 0; i < im.rows; i++){
        for(int j = 0; j < im.cols; j++){
            newImage.setPixelVal3(j, im.rows-i-1, 0, im.getPixelVal3(i,j,0));
            newImage.setPixelVal3(j, im.rows-i-1, 1, im.getPixelVal3(i,j,1));
            newImage.setPixelVal3(j, im.rows-i-1, 2, im.getPixelVal3(i,j,2));
        }
    }
    displayRGB(im);
    displayRGB(newImage);    
}

void brightNess(                Image &im, int level){
    Image newImage(im.rows, im.cols, im.depth);
    int pixel = 0, val = 0;
    for(int i = 0; i < im.rows; i++){
        for(int j = 0; j < im.cols; j++){
            newImage.setPixelVal(i, j, max(0, min(255, im.getPixelVal(i,j) + level)));
        }
    }
    displayGray(im);
    displayGray(newImage);
}
void Filtering(                 Image &im, double r,   double g, double b){
    Image newImage = Image(im.rows, im.cols, im.depth);
    int val = 0; 
    int pixel = 0;
    for(int i = 0; i < im.rows; i++){
        for(int j = 0; j < im.cols; j++){
            newImage.setPixelVal3(i, j, 0, min(255, (int)(im.getPixelVal3(i, j, 0)*r)));
            newImage.setPixelVal3(i, j, 1, min(255, (int)(im.getPixelVal3(i, j, 0)*g)));
            newImage.setPixelVal3(i, j, 2, min(255, (int)(im.getPixelVal3(i, j, 0)*b)));
        }
    }
    displayRGB(im);
    displayRGB(newImage);
}
void linearContrastSaturation(  Image &im, int sMin,   int sMax){
    if ((sMin > sMax) || (sMin < minPixel(im)) || (sMax > maxPixel(im))){
        cout << "Invalid saturation values" << endl;
        exit(1);
    }  
    Image newImage = Image(im.rows, im.cols, im.depth);
    int val = 0, pixel = 0;

    for (int i = 0; i < im.rows; i++)
    {
        for (int j = 0; j < im.cols; j++)
        {
            pixel = im.getPixelVal(i, j);
            val = 255 * (pixel - sMin) / (sMax - sMin);
            val = val > 255 ? 255 : max(val, 0);
            im.setPixelVal(i, j, val);
        }
    }  
    displayRGB(im);
    displayGray(newImage);    
}
void Scaling(                   Image &im, double xscale, double yscale){
    int destW = im.cols * (int)xscale;
    int destH = im.rows * (int)yscale;

    Image newImage = Image(destH, destW, im.depth);
    for(double i = 0; i < destH; i++){
        for(double j = 0; j < destW; j++){
            newImage.setPixelVal3((int)i, (int)j, 0, im.getPixelVal3((int)(i/yscale), (int)(j/xscale), 0));
            newImage.setPixelVal3((int)i, (int)j, 1, im.getPixelVal3((int)(i/yscale), (int)(j/xscale), 1));
            newImage.setPixelVal3((int)i, (int)j, 2, im.getPixelVal3((int)(i/yscale), (int)(j/xscale), 2));
        }
    }
    displayRGB(im);
    displayRGB(newImage);
}
