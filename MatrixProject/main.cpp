#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<memory>
#include"matrix.hpp"

using namespace std;

void print_matrix(const my::matrix<int>& m)
{
	for (const auto& arr : m)
	{
		for (const int& i : arr)
		{
			cout << i << endl;
		}
	}
}

int main(int argc, char** argv)
{
	system("chcp 1251");

	my::matrix<int> mtx(10, 10);

	for (int i = 0; i < (mtx.size()).col; ++i)
		mtx[0][i] = i + 1;

	for (int i = 1; i < (mtx.size()).row; ++i)
		mtx[i] = mtx[i - 1];

	mtx.resize(15, 20, 1);

	print_matrix(mtx);

	system("pause");
	return 0;
}