#include<iostream>
#include<string>
#include<vector>
#include "Observer.hpp"

class Apple : public ObservableObject<Apple>
{
public:
	Apple(){}
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
	Human(){}

	virtual void handle_event(const Apple& apple) override
	{
		std::cout << "Apple name changed: " << apple.get_name() << std::endl;
	}
};

template<typename T>
class Row
{
public:
	Row(T* data, int size) : data_{data}, size_{size} {}

	int size() const { return size_; }
	T* data() { return data_; }

	T* begin() { return data_; }
	T* end() { return (data_ + size_); }

	T const* cbegin() const { return data_; }
	T const* cend() const { return (data_ + size_); }

	std::vector<T> to_std_vector()
	{
		std::vector<T> result;
		result.reserve(size_);
		std::copy(data_, (data_ + size_), std::back_inserter(result));
		return result;
	}

private:
	bool valid_{ true };
	int size_{};
	T* data_{nullptr};
};

int main(int argc, char* argv[])
{
	using namespace std;

	system("chcp 1251");

/* 
	// Example using template Observer
	Apple appl("€блоко1");
	Human chel;

	appl.add_observer(chel);

	appl.change_name("новое €блоко");
*/

	Row<int> row(new int[10], 10); // WARNING: leak of memory!
	int counter{ 0 };
	for (auto& i : row)
	{
		i = ++counter;
	}

	for (auto i : row.to_std_vector())
	{
		cout << i << " ";
	}
	cout << endl;

	system("pause");
	return 0;
}