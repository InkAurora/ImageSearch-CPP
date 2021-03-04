#include "ImageSearch.h"

int main() {
	string imagePath;
	int x = 0, y = 0;
	int left, top, right, bottom, tolerance, ignore;

	cout << "Insert image path: ";
	cin >> imagePath;
	cout << "Insert left: ";
	cin >> left;
	cout << "Insert top: ";
	cin >> top;
	cout << "Insert right: ";
	cin >> right;
	if (right == 0) {
		right = GetSystemMetrics(SM_CXSCREEN);
	}
	cout << "Insert bottom: ";
	cin >> bottom;
	if (bottom == 0) {
		bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	cout << "Insert tolerance: ";
	cin >> tolerance;
	cout << "Insert hex color code to ignore: ";
	cin >> hex >> ignore;

	int success = ImageSearch(x, y, left, top, right, bottom, imagePath, tolerance, ignore);

	cout << success << " " << x << " " << y << endl;
	cin >> left;

	return 0;
}