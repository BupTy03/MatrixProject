#include<iostream>
#include<numeric>
#include<vector>
#include<array>
#include"matrix.hpp"
#include"SimpleTimer.hpp"
#include"TestObject.hpp"

int main(int argc, char** argv)
{
	system("chcp 1251");
	system("cls");

	my::SimpleTimer<std::chrono::nanoseconds> timer;

	try {
		timer.start();

		my::matrix<int> mtx(0, 4);

		int arr[4];
		std::iota(std::begin(arr), std::end(arr), 0);

		mtx.add_row(arr);
		mtx.add_row(arr);
		mtx.add_row(arr);

		mtx.row(2) = { -1, -1, -1, -1 };

		std::cout << mtx << std::endl;

		for (auto rit = mtx.rcbegin(); rit != mtx.rcend(); ++rit)
		{
			for (auto it = rit.base()->begin(); it != rit.base()->end(); ++it)
			{
				std::cout << *it << std::endl;
			}
		}

		mtx.swap_cols(0, 3);

		std::cout << mtx << std::endl;

		my::matrix<int> nullmtx(mtx.size(), 0);

		std::cout << nullmtx << std::endl;

		nullmtx.swap(mtx);

		std::cout << nullmtx << std::endl;

		timer.stop();
		timer.log_curr_time();
	}
	catch (const std::exception& exc) {
		std::cout << "Exception is caught: " <<  exc.what() << std::endl;
	}
	catch (...) {
		std::cout << "Something is going wrong\n";
	}

	system("pause");
	return 0;
}