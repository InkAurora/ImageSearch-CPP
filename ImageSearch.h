#include <iostream>
#include <stdlib.h>
#include <windows.h>
#include <vector>

#pragma once

using namespace std;

string ErrLvl = "";

int tolerance = 0;
int _left, _right, _top, _bottom;
int iR = 205;
int iG, iB;

typedef struct BMP {

	HDC hdc, hdcTemp;
	BYTE *bitPointer;
	int MAX_WIDTH, MAX_HEIGHT, mapSize, bitAlloc;
	BITMAPINFO bitmap;
	HBITMAP hBitmap2;

	void doMagic() {
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

		return;
	}

	void undoMagic() {
		DeleteObject(hdcTemp);
		ReleaseDC(NULL, hdcTemp);
		DeleteObject(hdc);
		ReleaseDC(NULL, hdc);
		DeleteObject(bitPointer);
		return;
	}

	void refreshMagic() {
		BitBlt(hdcTemp, 0, 0, MAX_WIDTH, MAX_HEIGHT, hdc, 0, 0, SRCCOPY);
		return;
	}

} BMP;

typedef struct BMP_IMAGE {

	int MAX_WIDTH, MAX_HEIGHT, mapSize, bitAlloc;
	BITMAP bm;
	HBITMAP hBMP;
	BITMAPINFOHEADER bmInfoHeader;
	unsigned char* bitPointer;
	
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

		return 1;
	}

	void deleteBMP() {
		DeleteObject(hBMP);
		delete [] bitPointer;

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
	BYTE* screen = bmp.bitPointer;
	BYTE* image = bmpImage.bitPointer;
	int height, sHeight, width, sWidth;
	for (int i = 0; i < bmpImage.MAX_HEIGHT; i++) {
		height = i * bmpImage.MAX_WIDTH * bmpImage.bitAlloc;
		sHeight = (y + i) * bmp.MAX_WIDTH * bmp.bitAlloc;
		for (int j = 0; j < bmpImage.MAX_WIDTH; j++) {
			if (x + j > _right) return 0;
			if (y + i > _bottom) return 0;

			int sPos = sHeight + ((x + j) * bmp.bitAlloc);
			int pos = height + (j * bmpImage.bitAlloc);

			if (iR != 205) {
				if (image[pos + 0] == iB and
					image[pos + 1] == iG and
					image[pos + 2] == iR) continue;
			}

			if (abs(screen[sPos + 0] - image[pos + 0]) > tolerance) return 0;
			if (abs(screen[sPos + 1] - image[pos + 1]) > tolerance) return 0;
			if (abs(screen[sPos + 2] - image[pos + 2]) > tolerance) return 0;
		}
	}

	return 1;
}

int FindPixelMatch(BMP bmp, BMP_IMAGE bmpImage) {
	BYTE* screen = bmp.bitPointer;
	BYTE* image = bmpImage.bitPointer;
	if (_right == GetSystemMetrics(SM_CXSCREEN)) _right -= 1;
	if (_bottom == GetSystemMetrics(SM_CYSCREEN)) _bottom -= 1;
	int sHeight;
	for (int i = _top; i <= _bottom; i++) {
		sHeight = i * bmp.MAX_WIDTH * bmp.bitAlloc;
		for (int j = _left; j <= _right; j++) {
			if (abs(image[0] - screen[sHeight + (j * bmp.bitAlloc) + 0]) > tolerance) continue;
			if (abs(image[1] - screen[sHeight + (j * bmp.bitAlloc) + 1]) > tolerance) continue;
			if (abs(image[2] - screen[sHeight + (j * bmp.bitAlloc) + 2]) <= tolerance) {
				if (CompareImage(bmp, bmpImage, j, i)) return sHeight + j * bmp.bitAlloc;
			}
		}
	}

	return 0;
}

int Test(BMP bmp, BMP_IMAGE bmpImage, int &x, int &y) {
	int position = FindPixelMatch(bmp, bmpImage);

	if (position > 0) {
		x = position / bmp.bitAlloc % bmp.MAX_WIDTH;
		y = position / bmp.bitAlloc / bmp.MAX_WIDTH;
		return 1;
	}

	return 0;
}

int ImageSearch(int &x, int &y, int left, int top, int right, int bottom, int tol, string imgPath = "", int ignoreColor = 0) {
	if (imgPath != "") {
		tolerance = tol;
		_left = left;
		_right = right;
		_top = top;
		_bottom = bottom;
		BMP screen;
		BMP_IMAGE image;
		screen.doMagic();
		image.createBMP(LoadPicture(imgPath));
	
		if (ignoreColor != 0) {
			iR = (ignoreColor >> 16) & 0xff;
			iG = (ignoreColor >> 8) & 0xff;
			iB = ignoreColor & 0xff;
		}

		int success = Test(screen, image, x, y);

		screen.undoMagic();
		image.deleteBMP();

		if (success) return 1;

		return 0;
	}

	ErrLvl = "2: Image path not specified.";
	return 2;
}
