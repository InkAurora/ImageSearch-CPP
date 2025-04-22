#include <iostream>
#include "FreeImage.h"

void generateTestImage(const char* filename, int width, int height) {
    FIBITMAP* bitmap = FreeImage_Allocate(width, height, 32);
    if (!bitmap) {
        std::cout << "Failed to create bitmap: " << filename << std::endl;
        return;
    }

    // Create a gradient pattern with a diagonal line
    RGBQUAD color;
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            color.rgbRed = (x * 255) / width;
            color.rgbGreen = (y * 255) / height;
            color.rgbBlue = 128;
            color.rgbReserved = 255; // Alpha

            // Add a white diagonal line
            if (x == y || x == y + 1 || x == y - 1) {
                color.rgbRed = color.rgbGreen = color.rgbBlue = 255;
            }

            FreeImage_SetPixelColor(bitmap, x, y, &color);
        }
    }

    if (FreeImage_Save(FIF_PNG, bitmap, filename)) {
        std::cout << "Successfully created test image: " << filename << std::endl;
    } else {
        std::cout << "Failed to save test image: " << filename << std::endl;
    }

    FreeImage_Unload(bitmap);
}

int main() {
    FreeImage_Initialise();

    generateTestImage("test_image_small.png", 100, 100);
    generateTestImage("test_image_medium.png", 400, 300);
    generateTestImage("test_image_large.png", 800, 600);

    FreeImage_DeInitialise();
    return 0;
}
