#pragma once

#include<set>
#include<algorithm>

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
	inline void add_observer(Observer<T>& obs);
	inline void delete_observer(Observer<T>& obs);

protected:
	virtual void notify();

private:
	std::set<Observer<T>*> observers;
};

template<typename T>
void ObservableObject<T>::add_observer(Observer<T>& obs)
{
	observers.insert(&obs);
}

template<typename T>
void ObservableObject<T>::delete_observer(Observer<T>& obs)
{
	observers.erase(&obs);
}

template<typename T>
void ObservableObject<T>::notify()
{
	std::for_each(std::begin(observers), std::end(observers),
	[this](auto p)
	{
		p->handle_event(*(dynamic_cast<T*>(this)));
	});
}