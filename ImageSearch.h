#include <iostream>
#include <stdlib.h>
#include <windows.h>
#include <vector>

#pragma once

using namespace std;

string ErrLvl = "";

int _tol = 0;
int _left, _right, _top, _bottom;
int _ignoreColor = 0xcccccccc;

typedef struct BMP {

	HDC hdc, hdcTemp;
	unsigned char* bitPointer;
	int MAX_WIDTH, MAX_HEIGHT, mapSize, bitAlloc;
	BITMAPINFO bitmap;
	HBITMAP hBitmap2;
	int* pixelMap;

	void createBMP() {
		hdc = GetDC(NULL);
		MAX_WIDTH = GetSystemMetrics(SM_CXSCREEN);
		MAX_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
		bitAlloc = 4;
		mapSize = MAX_WIDTH * MAX_HEIGHT * bitAlloc;

		bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
		bitmap.bmiHeader.biWidth = MAX_WIDTH;
		bitmap.bmiHeader.biHeight = -MAX_HEIGHT;
		bitmap.bmiHeader.biPlanes = 1;
		bitmap.bmiHeader.biBitCount = bitAlloc * 8;
		bitmap.bmiHeader.biCompression = BI_RGB;
		bitmap.bmiHeader.biSizeImage = mapSize;
		bitmap.bmiHeader.biClrUsed = 0;
		bitmap.bmiHeader.biClrImportant = 0;

		hBitmap2 = CreateDIBSection(hdcTemp, &bitmap, DIB_RGB_COLORS, (void**)(&bitPointer), NULL, 0);
		hdcTemp = CreateCompatibleDC(hdc);
		SelectObject(hdcTemp, hBitmap2);
		DeleteObject(hBitmap2);

		BitBlt(hdcTemp, 0, 0, MAX_WIDTH, MAX_HEIGHT, hdc, 0, 0, SRCCOPY);

		convertMap();

		return;
	}

	void deleteBMP() {
		DeleteObject(hdcTemp);
		ReleaseDC(NULL, hdcTemp);
		DeleteObject(hdc);
		ReleaseDC(NULL, hdc);
		delete[] pixelMap;
		pixelMap = 0;

		return;
	}

	void convertMap() {
		pixelMap = new int[mapSize / bitAlloc];
		int buffer;

		for (int i = 0; i < mapSize; i += bitAlloc) {
			buffer = int(bitPointer[i + 2] << 16 & 0xff0000);
			buffer += int(bitPointer[i + 1] << 8 & 0xff00);
			buffer += int(bitPointer[i + 0] & 0xff);
			pixelMap[i / bitAlloc] = buffer;
		}
		
		return;
	}

} BMP;

typedef struct BMP_IMAGE {

	int MAX_WIDTH, MAX_HEIGHT, mapSize, bitAlloc;
	BITMAP bm;
	HBITMAP hBMP;
	BITMAPINFOHEADER bmInfoHeader;
	unsigned char* bitPointer;
	int* pixelMap;
	
	int createBMP(HBITMAP bmp) {
		hBMP = bmp;
		GetObject(hBMP, sizeof(bm), &bm);
		MAX_WIDTH = bm.bmWidth;
		MAX_HEIGHT = bm.bmHeight;
		bitAlloc = 4;
		mapSize = MAX_HEIGHT * MAX_WIDTH * bitAlloc;

		::ZeroMemory(&bmInfoHeader, sizeof(BITMAPINFOHEADER));
		bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmInfoHeader.biWidth = MAX_WIDTH;
		bmInfoHeader.biHeight = -MAX_HEIGHT;
		bmInfoHeader.biPlanes = 1;
		bmInfoHeader.biBitCount = bitAlloc * 8;
		bmInfoHeader.biCompression = BI_RGB;
		bmInfoHeader.biSizeImage = mapSize;
		bmInfoHeader.biClrUsed = 0;
		bmInfoHeader.biClrImportant = 0;

		bitPointer = new unsigned char[mapSize];

		if (!GetDIBits(CreateCompatibleDC(0), hBMP, 0, MAX_HEIGHT, bitPointer, (BITMAPINFO*)&bmInfoHeader, DIB_RGB_COLORS)) {

			ErrLvl = "2: Could not create bitmap.";
			return 0;
		}

		convertMap();

		return 1;
	}

	void convertMap() {
		pixelMap = new int[mapSize / bitAlloc];
		int buffer;

		for (int i = 0; i < mapSize; i += bitAlloc) {
			buffer = int(bitPointer[i + 2] << 16 & 0xff0000);
			buffer += int(bitPointer[i + 1] << 8 & 0xff00);
			buffer += int(bitPointer[i + 0] & 0xff);
			pixelMap[i / bitAlloc] = buffer;
		}

		delete[] bitPointer;
		bitPointer = 0;

		return;
	}

	void deleteBMP() {
		DeleteObject(hBMP);
		delete[] pixelMap;
		pixelMap = 0;

		return;
	}

} BMP_IMAGE;

LPTSTR GetWorkingDir() {
	LPTSTR *buffer = new LPTSTR;
	GetCurrentDirectory(FILENAME_MAX + 1, *buffer);
	return *buffer;
}

HBITMAP LoadPicture(string img) {
	LPCSTR path = img.c_str();

	HBITMAP bmp = (HBITMAP) LoadImageA(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	if (!bmp) {
		ErrLvl = "2: Could not load bitmap.";
		return 0;
	}

	return bmp;
}

int CompareImage(BMP bmp, BMP_IMAGE bmpImage, int x, int y) {
	int* screen = bmp.pixelMap;
	int* image = bmpImage.pixelMap;
	int height, sHeight;
	for (int i = bmpImage.MAX_HEIGHT - 1; i >= 0; i--) {
		height = i * bmpImage.MAX_WIDTH;
		sHeight = (y + i) * bmp.MAX_WIDTH;
		for (int j = bmpImage.MAX_WIDTH - 1; j >= 0; j--) {
			if (x + j > _right) return 0;
			if (y + i > _bottom) return 0;

			if (image[height + j] == _ignoreColor) continue;

			if (_tol == 0) {
				if (image[height + j] != screen[sHeight + x + j]) return 0;
			} else {
				if (abs((image[height + j] >> 16) - (screen[sHeight + x + j] >> 16)) > _tol or
					abs((image[height + j] >> 8 & 0xff) - (screen[sHeight + x + j] >> 8 & 0xff)) > _tol or
					abs((image[height + j] & 0xff) - (screen[sHeight + x + j] & 0xff)) > _tol) return 0;
			}
		}
	}

	return 1;
}

int FindPixelMatch(BMP bmp, BMP_IMAGE bmpImage) {
	int* screen = bmp.pixelMap;
	int* image = bmpImage.pixelMap;
	if (_right == GetSystemMetrics(SM_CXSCREEN)) _right -= 1;
	if (_bottom == GetSystemMetrics(SM_CYSCREEN)) _bottom -= 1;
	int sHeight;
	int scanTrans = 0;
	if (image[0] == _ignoreColor) scanTrans = 1;
	for (int i = _top; i <= _bottom; i++) {
		sHeight = i * bmp.MAX_WIDTH;
		for (int j = _left; j <= _right; j++) {
			if (scanTrans) {
				if (CompareImage(bmp, bmpImage, j, i)) return sHeight + j;
			}
			if (_tol == 0) {
				if (image[0] == screen[sHeight + j]) {
					if (CompareImage(bmp, bmpImage, j, i)) return sHeight + j;
				}
			} else {
				if (abs((image[0] >> 16) - (screen[sHeight + j] >> 16)) <= _tol and
					abs((image[0] >> 8 & 0xff) - (screen[sHeight + j] >> 8 & 0xff)) <= _tol and
					abs((image[0] & 0xff) - (screen[sHeight + j] & 0xff)) <= _tol) {
					if (CompareImage(bmp, bmpImage, j, i)) return sHeight + j;
				}
			}
		}
	}

	return -1;
}

int FindImage(BMP bmp, BMP_IMAGE bmpImage, int &x, int &y) {
	int position = FindPixelMatch(bmp, bmpImage);

	if (position > -1) {
		x = position % bmp.MAX_WIDTH;
		y = position / bmp.MAX_WIDTH;
		return 1;
	}

	return 0;
}

int ImageSearch(int &x, int &y, int left, int top, int right, int bottom, string imgPath = "", int tol = 0, int ignoreColor = 0xcccccccc) {
	if (imgPath != "") {
		_left = left;
		_right = right;
		_top = top;
		_bottom = bottom;
		_tol = tol;
		if (_tol < 0) return 0;
		_ignoreColor = ignoreColor;
		BMP screen;
		BMP_IMAGE image;
		screen.createBMP();
		image.createBMP(LoadPicture(imgPath));

		int success = FindImage(screen, image, x, y);

		screen.deleteBMP();
		image.deleteBMP();

		if (success) return 1;

		return 0;
	}

	ErrLvl = "2: Image path not specified.";
	return 2;
}
