#include <iostream>
#include <stdlib.h>
#include <windows.h>

#pragma once

using namespace std;

string ErrLvl = "";

int _tol = 0;
int _left, _right, _top, _bottom;
int _ignoreColor = 0xcccccccc;

class BMP {
public:
  HDC hdc, hdcTemp;
  BITMAPINFO bitmapInfo;
  HBITMAP hBitmap;
  int MAX_WIDTH, MAX_HEIGHT, MAP_SIZE, BIT_ALLOC;
  int* pixelMap;
  unsigned char* bitPointer;

  BMP() {
	BIT_ALLOC = 4;
	MAX_WIDTH = GetSystemMetrics(SM_CXSCREEN);
	MAX_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
	MAP_SIZE = MAX_WIDTH * MAX_HEIGHT * BIT_ALLOC;

	hdc = GetDC(0);
	if (!hdc) return;

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = MAX_WIDTH;
	bitmapInfo.bmiHeader.biHeight = -MAX_HEIGHT;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = BIT_ALLOC * 8;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biSizeImage = MAP_SIZE;
	bitmapInfo.bmiHeader.biClrUsed = 0;
	bitmapInfo.bmiHeader.biClrImportant = 0;

	hBitmap = CreateDIBSection(hdc, &bitmapInfo, DIB_RGB_COLORS, (void**)(&bitPointer), NULL, 0);

	if (!hBitmap) return;

	hdcTemp = CreateCompatibleDC(NULL);

	if (!hdcTemp) return;

	SelectObject(hdcTemp, hBitmap);

	BitBlt(hdcTemp, 0, 0, MAX_WIDTH, MAX_HEIGHT, hdc, 0, 0, SRCCOPY);

	ConvertMap();

	return;
  }

  void DeleteBMP() {
	delete[] pixelMap;

	return;
  }

private:
  void ConvertMap() {
	int buffer;

	pixelMap = new int[MAP_SIZE / BIT_ALLOC];

	for (int i = 0; i < MAP_SIZE; i += BIT_ALLOC) {
	  buffer = int(bitPointer[i + 2] << 16 & 0xff0000);
	  buffer += int(bitPointer[i + 1] << 8 & 0xff00);
	  buffer += int(bitPointer[i + 0] & 0xff);
	  pixelMap[i / BIT_ALLOC] = buffer;
	}

	DeleteObject(hBitmap);
	ReleaseDC(NULL, hdc);
	DeleteDC(hdc);
	DeleteDC(hdcTemp);

	return;
  }
};

class BMP_IMAGE {
public:
  HDC hdc;
  BITMAP bitmap;
  HBITMAP hBitmap;
  BITMAPINFOHEADER bitmapInfo;
  int MAX_WIDTH, MAX_HEIGHT, MAP_SIZE, BIT_ALLOC;
  int* pixelMap;
  unsigned char* bitPointer;

  BMP_IMAGE(HBITMAP bmp) {
	if (!bmp) return;

	hBitmap = bmp;
	GetObject(hBitmap, sizeof(bitmap), &bitmap);

	if (!bitmap.bmWidth) return;

	MAX_WIDTH = bitmap.bmWidth;
	MAX_HEIGHT = bitmap.bmHeight;
	BIT_ALLOC = 4;
	MAP_SIZE = MAX_WIDTH * MAX_HEIGHT * BIT_ALLOC;

	bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.biWidth = MAX_WIDTH;
	bitmapInfo.biHeight = -MAX_HEIGHT;
	bitmapInfo.biPlanes = 1;
	bitmapInfo.biBitCount = BIT_ALLOC * 8;
	bitmapInfo.biCompression = BI_RGB;
	bitmapInfo.biSizeImage = MAP_SIZE;
	bitmapInfo.biClrUsed = 0;
	bitmapInfo.biClrImportant = 0;

	hdc = CreateCompatibleDC(NULL);

	bitPointer = new unsigned char[MAP_SIZE];

	if (!GetDIBits(hdc, hBitmap, 0, MAX_HEIGHT, bitPointer, (BITMAPINFO*)&bitmapInfo, DIB_RGB_COLORS)) return;

	ConvertMap();

	return;
  }

  void DeleteBMP() {
	delete[] pixelMap;

	return;
  }

private:
  void ConvertMap() {
	int buffer;

	pixelMap = new int[MAP_SIZE / BIT_ALLOC];

	for (int i = 0; i < MAP_SIZE; i += BIT_ALLOC) {
	  buffer = int(bitPointer[i + 2] << 16 & 0xff0000);
	  buffer += int(bitPointer[i + 1] << 8 & 0xff00);
	  buffer += int(bitPointer[i + 0] & 0xff);
	  pixelMap[i / BIT_ALLOC] = buffer;
	}

	DeleteObject(hBitmap);
	DeleteDC(hdc);
	delete[] bitPointer;

	return;
  }
};




LPTSTR GetWorkingDir() {
  LPTSTR* buffer = new LPTSTR;
  GetCurrentDirectory(FILENAME_MAX + 1, *buffer);
  return *buffer;
}

HBITMAP LoadPicture(string img) {
  LPCSTR path = img.c_str();

  HBITMAP bmp = (HBITMAP)LoadImageA(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

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
	  }
	  else {
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
	  }
	  else {
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

int FindImage(BMP bmp, BMP_IMAGE bmpImage, int& x, int& y) {
  int position = FindPixelMatch(bmp, bmpImage);

  if (position > -1) {
	x = position % bmp.MAX_WIDTH;
	y = position / bmp.MAX_WIDTH;
	return 1;
  }

  return position;
}

int ImageSearch(int& x, int& y, int left, int top, int right, int bottom, string imgPath = "", int tol = 0, int ignoreColor = 0xcccccccc) {
  if (imgPath != "") {
	_left = left;
	_right = right;
	_top = top;
	_bottom = bottom;
	_tol = tol;
	_ignoreColor = ignoreColor;

	if (_tol < 0) return 0;

	BMP screen = BMP();

	BMP_IMAGE image = BMP_IMAGE(LoadPicture(imgPath));

	int success = FindImage(screen, image, x, y);

	screen.DeleteBMP();
	image.DeleteBMP();

	if (success == 1) return 1;

	return 0;
  }

  ErrLvl = "2: Image path not specified.";
  return 2;
}
