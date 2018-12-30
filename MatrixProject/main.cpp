#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<memory>
#include"matrix.hpp"
#include"TestObject.hpp"
#include"SimpleTimer.hpp"

using namespace std;

int main(int argc, char** argv)
{
	system("chcp 1251");
	system("cls");

	my::SimpleTimer<std::chrono::milliseconds> timer;

	try {
		timer.start();

		my::matrix<my::TestObject> mtx_obj(3, 4);

		timer.stop();
		timer.log_curr_time();
	}
	catch (const std::exception& exc) {
		cout << exc.what() << endl;
	}

	system("pause");
	return 0;
}