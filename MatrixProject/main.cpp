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

	my::SimpleTimer<std::chrono::microseconds> timer;

	try {
		timer.start();

		my::matrix<int> mtx(3, 3);

		int counter{ 0 };
		for (auto& row : mtx)
		{
			for (auto& i : row)
			{
				i = ++counter;
			}
		}

		std::cout << mtx << std::endl;

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