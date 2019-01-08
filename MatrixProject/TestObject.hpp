#pragma once
#ifndef TEST_OBJECT_HPP
#define TEST_OBJECT_HPP

#include<iostream>
#include<string>
#include<vector>

namespace my
{
	class TestObject
	{
	public:
		TestObject() : id{ ++counter }
		{
			std::cout << "[object " << id << "] " << "TestObject::TestObject() (default constructor)" << std::endl;
			status = "constructed with default constructor";
			history.push_back(status);
		}

		TestObject(const TestObject& other)
			: id{ ++counter }, parent_id{ other.id }
		{
			std::cout << "[object " << id << "] " << "TestObject::TestObject(const TestObject& other) (copy constructor)" << std::endl;
			status = "constructed with copy constructor from object with id: ";
			status += std::to_string(other.id);
			history.push_back(status);

			// ----------- Exception testing ------
			if (id > 5)
				throw std::exception{ "TEST" };
			// ------------------------------------
		}
		TestObject& operator=(const TestObject& other)
		{
			if (this == &other) return *this;
			std::cout << "[object " << id << "] " << "TestObject& TestObject::operator=(const TestObject& other) (copy assignment operator)" << std::endl;
			status = "parameters of this object is assigned with parameters of object with id: ";
			status += std::to_string(other.id);
			history.push_back(status);
			return *this;
		}

		TestObject(TestObject&& other) noexcept
			: id{ ++counter }, parent_id{ other.id }
		{
			std::cout << "[object " << id << "] " << "TestObject::TestObject(TestObject&& other) (move constructor)" << std::endl;
			status = "constructed with move constructor from object with id: ";
			status += std::to_string(other.id);
			other.status = "is moved to new object with id: ";
			other.status += std::to_string(this->id);
			other.history.push_back(other.status);
			history.push_back(status);
		}
		TestObject& operator=(TestObject&& other) noexcept
		{
			if (this == &other) return *this;

			std::cout << "TestObject& TestObject::operator=(TestObject&& other) (move assignment operator)" << std::endl;
			other.status = "this object is moved to object with id: ";
			other.status += std::to_string(this->id);
			other.history.push_back(other.status);
			history.push_back(status);
			return *this;
		}

		virtual ~TestObject()
		{
			std::cout << "[object " << id << "] " << "virtual TestObject::~TestObject() (destructor)" << std::endl;
			status = "this object is destructed";
			history.push_back(status);
		}

		friend std::ostream& operator<<(std::ostream& os, const TestObject& obj)
		{
			os << "TestObject{id: " << obj.id << "}";
			return os;
		}

		bool operator==(const TestObject& other) const { return id == other.id; }
		bool operator!=(const TestObject& other) const { return id != other.id; }
		bool operator<(const TestObject& other) const { return id < other.id; }
		bool operator>(const TestObject& other) const { return id > other.id; }
		bool operator<=(const TestObject& other) const { return id <= other.id; }
		bool operator>=(const TestObject& other) const { return id >= other.id; }

		std::vector<std::string> getHistory() const { return history; }
		std::string getStatus() const { return status; }
		int getId() const { return id; }
		int getParentId() const { return parent_id; }

	protected:
		static int counter;
		int id{};
		int parent_id{};
		std::string status;
		std::vector<std::string> history;
	};

	int TestObject::counter = 0;
}

#endif // TEST_OBJECT_HPP
