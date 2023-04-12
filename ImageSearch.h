#include <iostream>
#include <stdlib.h>
#include <Windows.h>
#include "FreeImage.h"

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
	  buffer = int(bitPointer[i + 3] << 24 & 0xff000000);
	  buffer += int(bitPointer[i + 2] << 16 & 0xff0000);
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

int _imgWidth, _imgHeight;

// https://freeimage.sourceforge.io/

bool Load32Bitmap(const char* filename, int*& pixels) {
  FIBITMAP* dib = FreeImage_Load(FIF_BMP, filename, BMP_DEFAULT);

  if (!dib) return false;

  FIBITMAP* bitmap = FreeImage_ConvertTo32Bits(dib);
  FreeImage_Unload(dib);

  if (!bitmap) return false;

  _imgWidth = FreeImage_GetWidth(bitmap);
  _imgHeight = FreeImage_GetHeight(bitmap);
  int pitch = FreeImage_GetPitch(bitmap);

  // ARGB formatted ints
  // A = pixels[i] >> 24 & 0xff;
  // R = pixels[i] >> 16 & 0xff;
  // G = pixels[i] >> 8 & 0xff;
  // B = pixels[i] & 0xff;
  pixels = new int[_imgWidth * _imgHeight];
  FreeImage_ConvertToRawBits((BYTE*)pixels, bitmap, pitch, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

  FreeImage_Unload(bitmap);

  return true;
}




bool CompareImage(BMP bmp, int* bmpImage, int x, int y) {
  int* screen = bmp.pixelMap;
  int height, sHeight, tol;

  for (int i = _imgHeight - 1; i >= 0; i--) {
	height = i * _imgWidth;
	sHeight = (y + i) * bmp.MAX_WIDTH;

	for (int j = _imgWidth - 1; j >= 0; j--) {
	  tol = 0xff + _tol - (bmpImage[height + j] >> 24 & 0xff);
	  if (tol >= 0xff) continue;

	  if (x + j > _right) return false;
	  if (y + i > _bottom) return false;

	  if ((bmpImage[height + j] & 0xffffff) == _ignoreColor and (bmpImage[height + j] >> 24 & 0xff) == 0xff) continue;

	  if (tol == 0) {
		if (bmpImage[height + j] != screen[sHeight + x + j]) {
		  return false;
		}
	  }
	  else {
		if (abs((bmpImage[height + j] >> 16 & 0xff) - (screen[sHeight + x + j] >> 16 & 0xff)) > tol or
		  abs((bmpImage[height + j] >> 8 & 0xff) - (screen[sHeight + x + j] >> 8 & 0xff)) > tol or
		  abs((bmpImage[height + j] & 0xff) - (screen[sHeight + x + j] & 0xff)) > tol) {
		  return false;
		}
	  }
	}
  }

  return true;
}

int FindPixelMatch(BMP bmp, int* bmpImage) {
  int* screen = bmp.pixelMap;
  int scanTrans = 0;
  int nonTransBitPos = 0;
  int sHeight;
  int tol = _tol;

  if (_right == GetSystemMetrics(SM_CXSCREEN)) _right -= 1;
  if (_bottom == GetSystemMetrics(SM_CYSCREEN)) _bottom -= 1;

  bool done = 0;

  for (int i = 0; i < _imgHeight; i++) {
	nonTransBitPos = i * _imgWidth;

	for (int j = 0; j < _imgWidth; j++) {
	  if ((bmpImage[nonTransBitPos + j] & 0xffffff) == _ignoreColor) continue;
	  if ((bmpImage[nonTransBitPos + j] >> 24 & 0xff) > 0x00) {
		nonTransBitPos += j;
		done = true;
		break;
	  }
	}
	if (done) break;
  }

  tol += 0xff - (bmpImage[nonTransBitPos] >> 24 & 0xff);

  for (int i = _top; i <= _bottom; i++) {
	sHeight = i * bmp.MAX_WIDTH;

	for (int j = _left; j <= _right; j++) {
	  if (tol == 0) {
		if (bmpImage[nonTransBitPos] == screen[sHeight + j]) {
		  if (CompareImage(bmp, bmpImage, j - (nonTransBitPos % _imgWidth), i - (nonTransBitPos / _imgWidth))) {
			return (sHeight - ((nonTransBitPos / _imgWidth) * bmp.MAX_WIDTH)) + (j - (nonTransBitPos % _imgWidth));
		  }
		}
	  }
	  else {
		if (abs((bmpImage[nonTransBitPos] >> 16 & 0xff) - (screen[sHeight + j] >> 16 & 0xff)) <= tol and
		  abs((bmpImage[nonTransBitPos] >> 8 & 0xff) - (screen[sHeight + j] >> 8 & 0xff)) <= tol and
		  abs((bmpImage[nonTransBitPos] & 0xff) - (screen[sHeight + j] & 0xff)) <= tol) {
		  if (CompareImage(bmp, bmpImage, j - (nonTransBitPos % _imgWidth), i - (nonTransBitPos / _imgWidth))) {
			return (sHeight - ((nonTransBitPos / _imgWidth) * bmp.MAX_WIDTH)) + (j - (nonTransBitPos % _imgWidth));
		  }
		}
	  }
	}
  }

  return -1;
}

bool FindImage(BMP bmp, int* bmpImage, int& x, int& y) {
  int position = FindPixelMatch(bmp, bmpImage);

  if (position > -1) {
	x = position % bmp.MAX_WIDTH;
	y = position / bmp.MAX_WIDTH;

	return true;
  }

  return false;
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
	int* image;

	if (!Load32Bitmap(imgPath.c_str(), image)) {
	  screen.DeleteBMP();
	  delete[] image;

	  return 2;
	}

	bool result = FindImage(screen, image, x, y);

	screen.DeleteBMP();
	delete[] image;

	if (result) return 1;
	return 0;
  }

  ErrLvl = "2: Image path not specified.";
  return 2;
}
