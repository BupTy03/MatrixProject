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

	my::MatrixBase<int, allocator<int>> mtx(3, 4);

	my::matrix<int> mtx2(10, 10, 0);
	my::matrix<int>::row mtx2Row = mtx2.at_row(2);

	//mtx2Row.at(3) = 5;
	try
	{
		(mtx2.at_row(2)).at(1) = 5;
		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{
				std::cout << (mtx2.at_row(i)).at(j) << " ";
			}
			std::cout << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
	}

	/*for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			(mtx.data())[i][j] = i + j;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			std::cout << (mtx.data())[i][j] << " ";
		}
		std::cout << std::endl;
	}*/

	system("pause");
	return 0;
}

/*
	// Example using template Observer
	Apple appl("€блоко1");
	Human chel;

	appl.add_observer(chel);

	appl.change_name("новое €блоко");
*/

#if 0
class Apple : public ObservableObject<Apple>
{
public:
	Apple() {}
	Apple(const std::string& name) : name_(name) {}

	void change_name(const std::string& name)
	{
		name_ = name;
		notify();
	}

	std::string get_name() const
	{
		return name_;
	}

private:
	std::string name_;
};

class Human : public Observer<Apple>
{
public:
	Human() {}

	virtual void handle_event(const Apple& apple) override
	{
		std::cout << "Apple name changed: " << apple.get_name() << std::endl;
	}
};
#endif