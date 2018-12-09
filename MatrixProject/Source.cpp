#include<iostream>
#include<string>
#include<vector>
#include"Observer.hpp"
#include"Matrix.hpp"

template<typename T>
class Row
{
public:
	Row(T* data, int size) : data_{data}, size_{size} {}

	inline int size() const { return size_; }
	inline T* data() { return data_; }

	inline T* begin() { return data_; }
	inline T* end() { return (data_ + size_); }

	inline T const* cbegin() const { return data_; }
	inline T const* cend() const { return (data_ + size_); }

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

	Matrix<int> mtx(3, 3, 1);
	int** ptr = mtx.get_data();

	cout << "Mid: " << ptr[0][1] << endl;

	mtx.add_d1(std::vector<int>() = {1,2,4});
	mtx.add_d2();
	cout << mtx << endl;

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