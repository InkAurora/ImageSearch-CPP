# ImageSearch C++

C++ function to search an image on the screen and get its coordinates.

Currently only supports .bmp files.

## Instructions

To be able to use it, first import `ImageSearch.h`. ( *`main.cpp` is useless in this case, it's only useful for testing* )

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
- `imgPath` An string with the path of the image to be used in the search. e.g. `C:/someFolder/foo.bmp`.
- `tol` An integer tolerance value varying from `0` to `255`, the higher the value the higher the color tolerance when searching the image.
- `ignoreColor` An integer hex RGB color code to ignore. The pixels of the chosen color will match any pixel during the search. Useful for 'adding transparency'. e.g. The code `0xff0000` will make the search ignore every **RED(255, 0, 0)** pixel on the image. 

### Return Values

If the search succeeds, the `x` and `y` variables will be assigned with the coordinates where the image was found on the screen and the function will return `1`.

If the image was not found on the screen, the function will return `0`.

If the search could not be completed for some reason, the function will return `2`.
