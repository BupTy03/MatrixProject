#include<iostream>
#include<vector>
#include<array>
#include"matrix.hpp"
#include"SimpleTimer.hpp"

int main(int argc, char** argv)
{
	system("chcp 1251");
	system("cls");

	my::SimpleTimer<std::chrono::nanoseconds> timer;

	try {
		timer.start();
		
		my::matrix<int> mtx(3, 4);
		mtx[0] = { 1,2,3,4 };

		std::cout << mtx << std::endl;

		my::matrix<int> mtx2 = std::move(mtx);

		std::cout << mtx2 << std::endl;
		std::cout << mtx << std::endl;

		timer.stop();
		timer.log_curr_time();
	}
	catch (const std::exception& exc) {
		std::cout << exc.what() << std::endl;
	}

	system("pause");
	return 0;
}