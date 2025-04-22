#ifndef IMAGE_SEARCH_HPP
#define IMAGE_SEARCH_HPP

#include <iostream>
#include <string>
#include <Windows.h>
#include "FreeImage.h"

namespace ImageSearch {
    namespace detail {
        static std::string ErrLvl = "";
        static int _tol = 0;
        static int _left = 0, _right = 0, _top = 0, _bottom = 0;
        static int _ignoreColor = 0xcccccccc;
        static int _imgWidth = 0, _imgHeight = 0;

        class BMP {
        public:
            HDC hdc, hdcTemp;
            BITMAPINFO bitmapInfo;
            HBITMAP hBitmap, hOldBitmap;
            int MAX_WIDTH, MAX_HEIGHT, MAP_SIZE, BIT_ALLOC;
            int* pixelMap;
            unsigned char* bitPointer;

            BMP() : hdc(nullptr), hdcTemp(nullptr), hBitmap(nullptr), hOldBitmap(nullptr),
                   pixelMap(nullptr), bitPointer(nullptr) {
                try {
                    BIT_ALLOC = 4;
                    MAX_WIDTH = GetSystemMetrics(SM_CXSCREEN);
                    MAX_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
                    MAP_SIZE = MAX_WIDTH * MAX_HEIGHT * BIT_ALLOC;

                    hdc = GetDC(0);
                    if (!hdc) {
                        return;
                    }

                    ZeroMemory(&bitmapInfo, sizeof(BITMAPINFO));
                    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
                    bitmapInfo.bmiHeader.biWidth = MAX_WIDTH;
                    bitmapInfo.bmiHeader.biHeight = -MAX_HEIGHT;  // Top-down DIB
                    bitmapInfo.bmiHeader.biPlanes = 1;
                    bitmapInfo.bmiHeader.biBitCount = BIT_ALLOC * 8;
                    bitmapInfo.bmiHeader.biCompression = BI_RGB;
                    bitmapInfo.bmiHeader.biSizeImage = MAP_SIZE;

                    hBitmap = CreateDIBSection(hdc, &bitmapInfo, DIB_RGB_COLORS, (void**)(&bitPointer), NULL, 0);
                    if (!hBitmap) {
                        CleanupDC();
                        return;
                    }

                    hdcTemp = CreateCompatibleDC(NULL);
                    if (!hdcTemp) {
                        CleanupDC();
                        return;
                    }

                    hOldBitmap = (HBITMAP)SelectObject(hdcTemp, hBitmap);
                    if (!hOldBitmap) {
                        CleanupDC();
                        return;
                    }

                    if (!BitBlt(hdcTemp, 0, 0, MAX_WIDTH, MAX_HEIGHT, hdc, 0, 0, SRCCOPY)) {
                        CleanupDC();
                        return;
                    }
                    
                    ConvertMap();
                } catch (...) {
                    CleanupDC();
                    throw;
                }
            }

            ~BMP() {
                DeleteBMP();
                CleanupDC();
            }

            void DeleteBMP() {
                if (pixelMap) {
                    delete[] pixelMap;
                    pixelMap = nullptr;
                }
            }

        private:
            void ConvertMap() {
                if (!bitPointer) return;
                
                try {
                    if (pixelMap) {
                        delete[] pixelMap;
                        pixelMap = nullptr;
                    }
                    
                    pixelMap = new int[MAP_SIZE / BIT_ALLOC];
                    
                    #pragma omp parallel for
                    for (int i = 0; i < MAP_SIZE / BIT_ALLOC; i++) {
                        const int srcIdx = i * BIT_ALLOC;
                        pixelMap[i] = (bitPointer[srcIdx + 3] << 24) | 
                                    (bitPointer[srcIdx + 2] << 16) | 
                                    (bitPointer[srcIdx + 1] << 8) | 
                                    bitPointer[srcIdx];
                    }
                } catch (...) {
                    if (pixelMap) {
                        delete[] pixelMap;
                        pixelMap = nullptr;
                    }
                    throw;
                }
            }

            void CleanupDC() {
                if (hdcTemp && hOldBitmap) {
                    SelectObject(hdcTemp, hOldBitmap);
                    hOldBitmap = nullptr;
                }
                if (hBitmap) {
                    DeleteObject(hBitmap);
                    hBitmap = nullptr;
                }
                if (hdcTemp) {
                    DeleteDC(hdcTemp);
                    hdcTemp = nullptr;
                }
                if (hdc) {
                    ReleaseDC(NULL, hdc);
                    hdc = nullptr;
                }
                bitPointer = nullptr;  // Don't delete - owned by DIB
            }
        };

        inline bool CompareImage(const BMP& bmp, const int* bmpImage, int x, int y) {
            if (x + _imgWidth > _right || y + _imgHeight > _bottom) {
                return false;
            }
            
            const int* screen = bmp.pixelMap;
            const int screenWidth = bmp.MAX_WIDTH;
            volatile bool hasError = false;
            
            #pragma omp parallel for
            for (int i = 0; i < _imgHeight; i++) {
                if (hasError) continue;
                
                const int imgRowOffset = i * _imgWidth;
                const int screenRowOffset = (y + i) * screenWidth + x;
                
                for (int j = 0; j < _imgWidth; j++) {
                    const int imgPixel = bmpImage[imgRowOffset + j];
                    const int screenPixel = screen[screenRowOffset + j];
                    
                    const int alpha = (imgPixel >> 24) & 0xff;
                    const int tol = 0xff + _tol - alpha;
                    if (tol >= 0xff) continue;
                    
                    if ((imgPixel & 0xffffff) == _ignoreColor && alpha == 0xff) continue;
                    
                    if (tol == 0) {
                        if (imgPixel != screenPixel) {
                            #pragma omp critical
                            hasError = true;
                            break;
                        }
                        continue;
                    }
                    
                    const int imgR = (imgPixel >> 16) & 0xff;
                    const int imgG = (imgPixel >> 8) & 0xff;
                    const int imgB = imgPixel & 0xff;
                    const int scrR = (screenPixel >> 16) & 0xff;
                    const int scrG = (screenPixel >> 8) & 0xff;
                    const int scrB = screenPixel & 0xff;
                    
                    if (abs(imgR - scrR) > tol || 
                        abs(imgG - scrG) > tol || 
                        abs(imgB - scrB) > tol) {
                        #pragma omp critical
                        hasError = true;
                        break;
                    }
                }
            }
            
            return !hasError;
        }

        inline bool LoadImageBits(const char* filename, int*& pixels) {
            try {
                static bool freeImageInitialized = false;
                if (!freeImageInitialized) {
                    FreeImage_Initialise();
                    freeImageInitialized = true;
                }

                if (!filename) {
                    return false;
                }

                FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);

                if (format == FIF_UNKNOWN) {
                    format = FreeImage_GetFIFFromFilename(filename);
                    if (format == FIF_UNKNOWN) {
                        return false;
                    }
                }

                FIBITMAP* dib = FreeImage_Load(format, filename, 0);
                if (!dib) {
                    return false;
                }

                FIBITMAP* bitmap = FreeImage_ConvertTo32Bits(dib);
                FreeImage_Unload(dib);

                if (!bitmap) {
                    return false;
                }

                _imgWidth = FreeImage_GetWidth(bitmap);
                _imgHeight = FreeImage_GetHeight(bitmap);
                
                int pitch = FreeImage_GetPitch(bitmap);
                pixels = new int[_imgWidth * _imgHeight];
                
                FreeImage_ConvertToRawBits(
                    (BYTE*)pixels,
                    bitmap,
                    pitch,
                    32,
                    FI_RGBA_RED_MASK,
                    FI_RGBA_GREEN_MASK,
                    FI_RGBA_BLUE_MASK,
                    TRUE
                );

                FreeImage_Unload(bitmap);
                return true;
            } catch (...) {
                return false;
            }
        }

        inline int FindPixelMatch(BMP& bmp, int* bmpImage) {
            try {
                const int* screen = bmp.pixelMap;
                const int screenWidth = bmp.MAX_WIDTH;
                
                int nonTransBitPos = -1;
                int firstValidPixel = 0;
                
                #pragma omp critical
                {
                    for (int i = 0; i < _imgHeight && nonTransBitPos == -1; i++) {
                        for (int j = 0; j < _imgWidth; j++) {
                            const int pixelPos = i * _imgWidth + j;
                            const int pixel = bmpImage[pixelPos];
                            if ((pixel & 0xffffff) != _ignoreColor && 
                                (pixel >> 24 & 0xff) > 0x00) {
                                nonTransBitPos = pixelPos;
                                firstValidPixel = pixel;
                                break;
                            }
                        }
                    }
                }
                
                if (nonTransBitPos == -1) {
                    return -1;
                }
                
                const int tol = _tol + 0xff - ((firstValidPixel >> 24) & 0xff);
                const int searchStartX = nonTransBitPos % _imgWidth;
                const int searchStartY = nonTransBitPos / _imgWidth;
                volatile int foundPosition = -1;
                
                #pragma omp parallel for ordered schedule(dynamic)
                for (int i = _top; i <= _bottom; i++) {
                    if (foundPosition != -1) continue;
                    
                    for (int j = _left; j <= _right; j++) {
                        if (foundPosition != -1) continue;
                        
                        const int screenPixel = screen[i * screenWidth + j];
                        bool matches = false;
                        
                        if (tol == 0) {
                            matches = (firstValidPixel == screenPixel);
                        } else {
                            const int imgR = (firstValidPixel >> 16) & 0xff;
                            const int imgG = (firstValidPixel >> 8) & 0xff;
                            const int imgB = firstValidPixel & 0xff;
                            const int scrR = (screenPixel >> 16) & 0xff;
                            const int scrG = (screenPixel >> 8) & 0xff;
                            const int scrB = screenPixel & 0xff;
                            
                            matches = (abs(imgR - scrR) <= tol &&
                                     abs(imgG - scrG) <= tol &&
                                     abs(imgB - scrB) <= tol);
                        }
                        
                        if (matches) {
                            const int offsetX = j - searchStartX;
                            const int offsetY = i - searchStartY;
                            
                            #pragma omp critical
                            {
                                if (foundPosition == -1 && CompareImage(bmp, bmpImage, offsetX, offsetY)) {
                                    foundPosition = offsetX + offsetY * screenWidth;
                                }
                            }
                        }
                    }
                }
                
                return foundPosition;
            } catch (...) {
                throw;
            }
        }

        inline bool FindImage(BMP& bmp, int* bmpImage, int& x, int& y) {
            int position = FindPixelMatch(bmp, bmpImage);

            if (position > -1) {
                x = position % bmp.MAX_WIDTH;
                y = position / bmp.MAX_WIDTH;
                return true;
            }

            return false;
        }

    }  // namespace detail

    /**
     * Search for an image on the screen
     * @param x Output parameter for found X coordinate
     * @param y Output parameter for found Y coordinate
     * @param left Left boundary of search area
     * @param top Top boundary of search area
     * @param right Right boundary of search area
     * @param bottom Bottom boundary of search area
     * @param imgPath Path to the image file to search for
     * @param tol Color tolerance (0-255)
     * @param ignoreColor Color to ignore in the search (default: 0xcccccccc)
     * @return 1 if found, 0 if not found, 2 if error
     */
    inline int Search(int& x, int& y, int left, int top, int right, int bottom, 
                     const std::string& imgPath, int tol = 0, int ignoreColor = 0xcccccccc) {
        try {
            if (imgPath.empty()) {
                detail::ErrLvl = "2: Image path not specified.";
                return 2;
            }

            detail::_left = left;
            detail::_right = right;
            detail::_top = top;
            detail::_bottom = bottom;
            detail::_tol = tol;
            detail::_ignoreColor = ignoreColor;

            if (tol < 0) return 0;

            detail::BMP screen;
            if (!screen.pixelMap) {
                return 2;
            }

            int* image = nullptr;
            bool result = false;
            
            try {
                if (!detail::LoadImageBits(imgPath.c_str(), image)) {
                    return 2;
                }

                result = detail::FindImage(screen, image, x, y);
            }
            catch (...) {
                if (image) {
                    delete[] image;
                }
                throw;
            }

            if (image) {
                delete[] image;
            }

            return result ? 1 : 0;
        } catch (...) {
            return 2;
        }
    }
}  // namespace ImageSearch

#endif  // IMAGE_SEARCH_HPP