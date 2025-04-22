# ImageSearch C++

C++ function to search an image on the screen and get its coordinates.

Supports .bmp and .png files. Other image formats may work, but have not been tested.

## Instructions

- To be able to use it, first import `ImageSearch.hpp`. ( _`main.cpp` provides example usage and testing capabilities_ ).
- Add `FreeImage.h`, `FreeImage.lib` and `FreeImage.dll` to your project dependencies.
  https://freeimage.sourceforge.io/

### Usage Examples

The project includes several ways to use and test the ImageSearch functionality:

1. Basic image search:

```cpp
int x = 0, y = 0;
int result = ImageSearch::Search(x, y, 0, 0, 1920, 1080, "path/to/image.png");
```

2. Performance testing:

```bash
ImageSearch.exe --perf-test
```

The project includes utilities for generating test images and running performance benchmarks.

### Syntax

```
int ImageSearch(
  int &x,
  int &y,
  int left,
  int top,
  int right,
  int bottom,
  string imgPath,
  int tol,
  int ignoreColor
);
```

### Parameters

- `&x` A reference to an `int` variable where will be stored the x coordinates of the image if found.
- `&y` A reference to an `int` variable where will be stored the y coordinates of the image if found.
- `left` The left x coordinate of the area of the search.
- `top` The top y coordinate of the area of the search.
- `right` The right x coordinate of the area of the search.
- `bottom` The bottom y coordinate of the area of the search.
- `imgPath` A string with the path of the image to be used in the search. e.g. `C:/SomeFolder/foo.bmp` or `C:\\SomeFolder\\foo.png`.
- `tol` An integer tolerance value varying from `0` to `255`. The higher the value, the higher the color shift tolerance when searching the image.
- `ignoreColor` An integer hex RGB color code to ignore. The pixels of the chosen color will match any pixel during the search. Useful for 'adding transparency'. e.g. The code `0xff0000` will make the search ignore every **RED(255, 0, 0)** pixel on the image.

### Return Values

If the search succeeds, the `x` and `y` variables will be assigned with the coordinates where the image was found on the screen and the function will return `1`.

If the search area is not the entire screen, coordinates will be relative to that area. To obtain absolute coordinates, simply add your `left` and `top` parameters to the relative coordinates.

If the image was not found on the screen, the function will return `0`.

If the search could not be completed for some reason, the function will return `2`.

## Testing

The project includes two types of tests:

1. Performance Tests: Run with `--perf-test` command line argument to benchmark the image search functionality
2. Test Image Generation: The project includes utilities to generate test images of various sizes for testing purposes

Test images are provided in three sizes:

- test_image_small.png
- test_image_medium.png
- test_image_large.png
