#ifndef IMAGE_H
#define IMAGE_H

class Image{
    public:
    Image();
    Image(int numRows, int numCols, int depth);
    ~Image();
    int   getPixelVal(int row, int col);
    void  setPixelVal(int row, int col, int value);
    int   getPixelVal3(int row, int col, int depth);
    void  setPixelVal3(int row, int col, int depth, int value);   
    int rows;
    int cols;
    int depth;
    int **pixelVal;
    int ***pixelVal3; 
};

#endif