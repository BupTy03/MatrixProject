#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<memory>
#include"matrix.hpp"

int main(int argc, char** argv)
{
	using namespace std;

	system("chcp 1251");

	my::matrix<int> mtx(10, 10);

	for (int i = 0; i < (mtx.size()).col; ++i)
		mtx[0][i] = i + 1;

	for (int i = 1; i < (mtx.size()).row; ++i)
		mtx[i] = mtx[i - 1];

	mtx.resize(15, 20, 1);

	cout << mtx << "\n" << endl;

	mtx[5] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	cout << mtx << endl;


	system("pause");
	return 0;
}