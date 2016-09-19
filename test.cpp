#include <iostream>
#include <fstream>
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;

struct imageHeader
{
	int magicNumber;
	int number;
	int row;
	int col;
};

struct labelHeader
{
	int magicNumber;
	int number;
};

int bytes2int(const char *bytes);
void readImageHeader(ifstream &infile, imageHeader &header);
void readLabelHeader(ifstream &infile, labelHeader &header);
Mat readImage(ifstream &infile, const imageHeader &ih, int index);
int readLabel(ifstream &infile, const labelHeader &header, int index);

int main()
{
	ifstream imagefile, labelfile;
	imagefile.open("train-images-idx3-ubyte", ifstream::in|ifstream::binary);
	labelfile.open("train-labels-idx1-ubyte", ifstream::in|ifstream::binary);
	imageHeader ih;
	labelHeader lh;
	readImageHeader(imagefile, ih);
	readLabelHeader(labelfile, lh);

	namedWindow("image", CV_WINDOW_NORMAL);
	for(int i=0; i<10; i++){
		imshow("image", readImage(imagefile,ih, i));
		waitKey(500);
		cout<<readLabel(labelfile, lh, i)<<" ";
	}
	imagefile.close();
	labelfile.close();

	// infile.close();
	return 0;
}

int bytes2int(const char *bytes)
{
	int n=0;
	for(int i=0; i<sizeof(int); i++) n=n*256+(uchar)bytes[i];
	return n;
}

void readImageHeader(ifstream &infile, imageHeader &header)
{
	if(!infile.is_open()) return;
	infile.seekg(0);
	char a[4];
	infile.read(a, 4);header.magicNumber=bytes2int(a);
	infile.read(a, 4);header.number=bytes2int(a);
	infile.read(a, 4);header.row=bytes2int(a);
	infile.read(a, 4);header.col=bytes2int(a);
}

void readLabelHeader(ifstream &infile, labelHeader &header)
{
	if(!infile.is_open()) return;
	infile.seekg(0);
	char a[4];
	infile.read(a, 4);header.magicNumber=bytes2int(a);
	infile.read(a, 4);header.number=bytes2int(a);
}

Mat readImage(ifstream &infile, const imageHeader &ih, int index)
{
	if(!infile.is_open() || index>=ih.number) return Mat();

	int offset=16+ih.row*ih.col*index;
	infile.seekg(offset);
	Mat img(ih.row,ih.col,CV_8UC1, Scalar(0));
	char buffer[ih.row*ih.col];
	infile.read(buffer, ih.row*ih.col);
	for(int row=0; row<ih.row; row++){
		for(int col=0; col<ih.col; col++){
			img.at<uchar>(row, col)=buffer[row*ih.col+col];
		}
	}
	return img;
}

int readLabel(ifstream &infile, const labelHeader &header, int index)
{
	if(!infile.is_open() || index>=header.number) return -1;

	int offset=8+index;
	infile.seekg(offset);
	char s;
	infile.read(&s, 1);
	return (uchar)s;
}