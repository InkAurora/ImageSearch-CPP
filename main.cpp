#include "ImageSearch.h"

int main() {
	string imagePath;
	int x = 0, y = 0;
	int left, top, right, bottom;

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

	int success = ImageSearch(x, y, left, top, right, bottom, 0, imagePath);

	cout << success << " " << x << " " << y << endl;
	cin >> left;

	return 0;
}