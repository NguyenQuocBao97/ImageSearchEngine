#pragma once

#include "stdafx.h"
#include "resource.h"
#include <commdlg.h>
#include <ShlObj.h>
#include <Windowsx.h>
#include "opencv2\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include <iostream>
#include <string>
#include <shellapi.h>
#pragma comment(lib, "shell32")
using namespace cv;
using namespace std;

string string2String(string& input);
Mat histogram(Mat& srcImg, Mat& mask);
Mat describe(Mat& srcImg);
void OnBrowseCopyDestinate(HWND hDlg, LPCWSTR title, int EditBoxID);
void OnOpenCopyDestination(HWND hDlg, LPCWSTR title, int EditBoxID);
void _searchImage(Mat srcImg, string& queryFolderPath, vector<Mat>& outputVector, int limit = 5);
void makeCanvas(vector<Mat>& vecMat, int windowHeight, Mat& output);
void disableItem(HWND hDlg, bool enable = false);
void save2File(vector<Mat>& srcImg, string& path);
void eraseAllImage(string& path);