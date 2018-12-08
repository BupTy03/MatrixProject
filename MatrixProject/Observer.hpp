#pragma once
#include<algorithm>
#include<vector>

template<typename T>
class Observer
{
public:
	virtual void handle_event(const T&) = 0;
};

template<typename T>
class ObservableObject
{
public:
	virtual ~ObservableObject(){}

	void add_observer(Observer<T>* obs);
	void delete_observer(Observer<T>* obs);

protected:
	void notify();

private:
	std::vector<Observer<T>*> observers;
};

template<typename T>
void ObservableObject<T>::add_observer(Observer<T>* obs)
{
	if (obs == nullptr)
	{
		return;
	}

	auto beg_ = std::cbegin(observers);
	auto end_ = std::cend(observers);

	if (std::find(beg_, end_, obs) != end_)
	{
		return;
	}

	observers.push_back(&obs);
}

template<typename T>
void ObservableObject<T>::delete_observer(Observer<T>* obs)
{
	if (obs == nullptr)
	{
		return;
	}

	auto beg_ = std::cbegin(observers);
	auto end_ = std::cend(observers);

	observers.erase(std::remove(beg_, end_, &obs), end_);
}

template<typename T>
void ObservableObject<T>::notify()
{
	for (auto observer : observers)
	{
		observer->handle_event(*(dynamic_cast<T*>(this)));
	}
}