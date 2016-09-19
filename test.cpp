#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <mysql_error.h>
#include <cppconn/statement.h>

using namespace std;
using namespace cv;
using namespace sql;

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

void save2DB(const string imageFileName, const string labelFIleName, const string tableName, Statement *stmt);

int bytes2int(const char *bytes);
void readImageHeader(ifstream &infile, imageHeader &header);
void readLabelHeader(ifstream &infile, labelHeader &header);
Mat readImage(ifstream &infile, const imageHeader &ih, int index);
int readLabel(ifstream &infile, const labelHeader &header, int index);

int main()
{
	string url("tcp://127.0.0.1:3306");
	string user("root");
	string pwd("yin941031");

	mysql::MySQL_Driver *driver=NULL;
	Connection *con=NULL;
	Statement *stmt=NULL;

	try{
		driver = sql::mysql::get_driver_instance();
		con = driver->connect(url, user, pwd);
		stmt = con->createStatement();
		stmt->execute("use MNIST");
		save2DB("t10k-images-idx3-ubyte", "t10k-labels-idx1-ubyte", "test", stmt);

		delete con;
		delete stmt;
	}catch(SQLException &e){
		cout << "# ERR: SQLException in " << __FILE__;
		// cout << "(" << EXAMPLE_FUNCTION << ") on line " << __LINE__ << endl;
		/* Use what() (derived from std::runtime_error) to fetch the error message */
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		cout << "not ok 1 - examples/connect.php" << endl;

	}
	
	return 0;
}

void save2DB(const string imageFileName, const string labelFIleName, const string tableName, Statement *stmt)
{
	ifstream imagefile, labelfile;
	imagefile.open(imageFileName.c_str(), ifstream::in | ifstream::binary);
	labelfile.open(labelFIleName.c_str(), ifstream::in | ifstream::binary);

	imageHeader ih;
	labelHeader lh;
	readImageHeader(imagefile, ih);
	readLabelHeader(labelfile, lh);

	if(ih.number != lh.number) return;

	stmt->execute("create table if not exists "+tableName+"(id INT PRIMARY KEY, image_path VARCHAR(100), label INT)");

	int a[10]={0};
	for(int i=0; i<ih.number; i++){
		Mat img = readImage(imagefile, ih, i);
		int label = readLabel(labelfile, lh, i);
		a[label]++;
		char name[100];
		sprintf(name, "%s_%d_%d.jpg",tableName.c_str(), label, a[label]);
		string path=string("/home/yinshuo/Datasets/MNIST/image/")+name;
		imwrite(path,img);
		char order[200];
		string temp="insert into "+tableName+" values(%d, \""+path+"\", %d)";
		sprintf(order, temp.c_str(), i, label);
		stmt->execute(order);
	}

	imagefile.close();
	labelfile.close();

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