#include "stdafx.h"
#include "ImageSearchEngine.h"
extern HWND hwndDlg;
extern HWND _progressBar;
extern HINSTANCE hInst;
extern bool clear;
extern HANDLE startThread;
bool increaseProg = false;


// browse dialog box
void OnBrowseCopyDestinate(HWND hDlg, LPCWSTR title, int EditBoxID)
{
	TCHAR szDir[MAX_PATH];
	szDir[0] = '\0';
	BROWSEINFO bInfo;
	bInfo.hwndOwner = hDlg;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = title; // Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST pidl;
	pidl = SHBrowseForFolder(&bInfo);

	if (pidl != NULL)
	{
		SHGetPathFromIDList(pidl, szDir);
		SetWindowText(GetDlgItem(hDlg, EditBoxID), szDir);
	}
}

// Open dialog box
void OnOpenCopyDestination(HWND hDlg, LPCWSTR title, int EditBoxID) {
	OPENFILENAME ofn;
	WCHAR szFilter[] = L"PNG file\0*.png\0";
	WCHAR szPath[MAX_PATH];
	szPath[0] = '\0';

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = 128;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT3), szPath);
	}

}

// calculate histogram from flatten image
Mat histogram(Mat& srcImg, Mat& mask) {
	vector<Mat> bgr;
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	split(srcImg, bgr);
	Mat b_hist, g_hist, r_hist;

	calcHist(&bgr[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, 1, 0);
	calcHist(&bgr[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, 1, 0);
	calcHist(&bgr[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, 1, 0);

	Mat result;
	result.push_back(b_hist);
	result.push_back(g_hist);
	result.push_back(r_hist);

	normalize(result, result);

	result.reshape(0, 1);
	return result;
}

// cut image into 5 part and flatten
Mat describe(Mat& srcImg) {
	Mat HSVImage;
	cvtColor(srcImg, HSVImage, CV_BGR2HSV);
	
	Mat result;

	int h = HSVImage.rows, w = HSVImage.cols;
	int cX = w / 2, cY = h / 2;

	vector<vector<int>> segment = {
		{0, cX, 0, cY},
		{cX, w, 0, cY},
		{cX, w, cY, h},
		{0, cX, cY, h} 
	};
	int axesX = int(w*0.75) / 2, axesY = int(h*0.75) / 2;

	Mat ellipseMask = Mat::zeros(h, w, CV_8U);
	cv::Point p(cX, cY);
	ellipse(ellipseMask, p, { axesX, axesY }, 0, 0, 360, 255);
	for (auto i : segment) {
		Mat cornerMask = Mat::zeros(h, w, CV_8U);
		cv::Point a(i[0], i[1]), b(i[2], i[3]);
		cv::Rect tmp(a, b);
		rectangle(cornerMask, tmp, 255, -1);
		subtract(cornerMask, ellipseMask, cornerMask);

		Mat hist = histogram(HSVImage, cornerMask);
		result.push_back(hist);
	}
	Mat hist = histogram(HSVImage, ellipseMask);
	result.push_back(hist);
	result.reshape(0, 1);
	return result;
}

// calculate chisquare distance between 2 histograms
double chi2_distance(Mat& histA, Mat& histB) {
	return compareHist(histA, histB, CV_COMP_CHISQR_ALT);
}

// extract n smallest image with complexity O(n) 
void getNSmallest(vector<Mat>& originalMat, vector<double>& chiValue, int& limit) {
	for (int i = 0; i < limit; ++i) {
		int t = i;
		for (int j = i + 1; j < chiValue.size(); ++j) {
			if (chiValue[t] > chiValue[j]) {
				t = j;
			}
		}
		swap(chiValue[t], chiValue[i]);
		swap(originalMat[t], originalMat[i]);	
	}
	originalMat.erase(originalMat.begin() + limit, originalMat.end());
}

// use chiValue for sorting output image, cost O(n*n)
void sortedByChiValue(vector<Mat>& originalMat, vector<double>& ChiValue, int& limit) {
	for (int i = 0; i < ChiValue.size(); ++i) {
		for (int j = i + 1; j < ChiValue.size(); ++j) {
			if (ChiValue[i] > ChiValue[j]) {
				swap(ChiValue[i], ChiValue[j]);
				swap(originalMat[i], originalMat[j]);
			}
		}
	}
	originalMat.erase(originalMat.begin() + limit, originalMat.end());
}

// make a string have length which equals to its real length
string string2String(string& input) {

	string result;
	for (auto &i : input) {
		if (i == '\0')
			break;
		result.push_back(i);
	}
	return result;
}

// Open image, split to 5 parts, compute chi-square value
void  _searchImage(Mat srcImg, string& queryFolderPath,  vector<Mat>& outputVector, int limit) {
	vector<double> chiSquareArray;
	vector<String> nameList;
	string resizePath = string2String(queryFolderPath)+"\\*.png";
	glob(resizePath, nameList);
	if (nameList.size() < limit) {
		limit = nameList.size();
	}
	_progressBar = GetDlgItem(hwndDlg, IDC_PROGRESS1);

	SendMessage(_progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, nameList.size()));
	SendMessage(_progressBar, PBM_SETSTEP, (WPARAM)1, 0);
	srcImg = describe(srcImg);
	for (auto &i : nameList) {
		Mat imgHolder = imread(i, IMREAD_COLOR);
		Mat histHolder = describe(imgHolder);
		double d = chi2_distance(histHolder, srcImg);
		chiSquareArray.push_back(d);
		outputVector.push_back(imgHolder);
		increaseProg = true;
		SendMessage(_progressBar, PBM_STEPIT, 0, 0);
		
	}
	getNSmallest(outputVector, chiSquareArray, limit);
	//sortedByChiValue(outputVector, chiSquareArray, limit);
} 

// Merge N images into 1 single image and draw it by imshow()
void makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, Mat& canvasImage) {
	int nRows = 5;
	int N = vecMat.size();
	nRows = nRows > N ? N : nRows;
	int edgeThickness = 10;
	int imagesPerRow = ceil(double(N) / nRows);
	int resizeHeight = floor(2.0 * ((floor(double(windowHeight - edgeThickness) / nRows)) / 2.0)) - edgeThickness;
	int maxRowLength = 0;

	vector<int> resizeWidth;
	for (int i = 0; i < N;) {
		int thisRowLen = 0;
		for (int k = 0; k < imagesPerRow; k++) {
			double aspectRatio = double(vecMat[i].cols) / vecMat[i].rows;
			int temp = int(ceil(resizeHeight * aspectRatio));
			resizeWidth.push_back(temp);
			thisRowLen += temp;
			if (++i == N) break;
		}
		if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > maxRowLength) {
			maxRowLength = thisRowLen + edgeThickness * (imagesPerRow + 1);
		}
	}
	int windowWidth = maxRowLength;
	canvasImage = Mat::zeros(windowHeight, windowWidth, CV_8UC3);

	for (int k = 0, i = 0; i < nRows; i++) {
		int y = i * resizeHeight + (i + 1) * edgeThickness;
		int x_end = edgeThickness;
		for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
			int x = x_end;
			Rect roi(x, y, resizeWidth[k], resizeHeight);
			Size s = canvasImage(roi).size();
			// change the number of channels to three
			Mat target_ROI(s, CV_8UC3);
			if (vecMat[k].channels() != canvasImage.channels()) {
				if (vecMat[k].channels() == 1) {
					cvtColor(vecMat[k], target_ROI, CV_GRAY2BGR);
				}
			}
			else {
				vecMat[k].copyTo(target_ROI);
			}
			resize(target_ROI, target_ROI, s);
			if (target_ROI.type() != canvasImage.type()) {
				target_ROI.convertTo(target_ROI, canvasImage.type());
			}
			target_ROI.copyTo(canvasImage(roi));
			x_end += resizeWidth[k] + edgeThickness;
		}
	}
}


// Disable button while processing, prevent user from clicking leads to crash
void disableItem(HWND hDlg, bool enable) {
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), enable);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), enable);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON3), enable);
	EnableWindow(GetDlgItem(hDlg, IDC_START), enable);
	EnableWindow(GetDlgItem(hDlg, IDC_COMBO1), enable);
}

// save all query images to destination output folder
void save2File(vector<Mat>& srcImg, string& path) {
	if (clear)
		eraseAllImage(path);
	int c = 1;
	for (auto &i : srcImg) {
		Mat tmp;
		i.convertTo(tmp, CV_8U);
		string pathTemp = string2String(path) + "\\" + to_string(c++) + ".png";
		imwrite(pathTemp, tmp);
	}
}

// erase old image exist in output folder
void eraseAllImage(string& path) {
	vector<String> nameList;
	string resizePath = string2String(path) + "\\*.png";
	glob(resizePath, nameList);
	for (auto& i : nameList) {
		remove(i.c_str());
	}
}
