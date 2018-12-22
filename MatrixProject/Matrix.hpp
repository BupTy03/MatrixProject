#pragma once
#ifndef MATRIX_HPP
#define MATRIX_HPP

#include<memory>
#include<scoped_allocator>
#include<cassert>
#include"observer.hpp"

namespace my
{
	using Index = int;

	struct matrix_size
	{
		matrix_size() {}
		explicit matrix_size(Index x, Index y) : row{ x }, col{ y } {}
		Index row{};
		Index col{};
	};

	template<class T, class A>
	struct MatrixBase
	{
		using allocator_type = A;
		using row_allocator = typename std::allocator_traits<A>::template rebind_alloc<A::pointer>;

		MatrixBase() {}
		MatrixBase(Index x, Index y) : sz{ x,y } 
		{ construct(x, y); }

		MatrixBase(const allocator_type& al_) : alloc{ row_allocator(), al_} {}
		MatrixBase(Index x, Index y, const allocator_type& al_)
			: sz(x, y), space(x, y), alloc(row_allocator(), al_)
		{ construct(x, y); }

		~MatrixBase()
		{
			for (Index i = 0; i < sz.row; ++i)
				deallocate_columns(elem[i], sz.col);

			deallocate_rows(elem, sz.row);
		}

		allocator_type get_allocator() { return alloc.inner_allocator(); }

		T** data() { return elem; }

	protected:
		T** allocate_rows(Index x)
		{
			return x > 0 ? alloc.allocate(x) : nullptr;
		}

		T* allocate_columns(Index y)
		{
			return y > 0 ? (alloc.inner_allocator()).allocate(y) : nullptr;
		}

		void deallocate_rows(T** p, Index x)
		{
			if (p == nullptr)
				return;

			alloc.deallocate(p, x);
		}

		void deallocate_columns(T* p, Index y)
		{
			if (p == nullptr)
				return;

			(alloc.inner_allocator()).deallocate(p, y);
		}

	private:
		void construct(Index x, Index y)
		{
			elem = allocate_rows(x);
			for (Index i = 0; i < x; ++i)
				elem[i] = allocate_columns(y);
		}

	protected:
		std::scoped_allocator_adaptor<row_allocator, allocator_type> alloc;
		T** elem{};
		matrix_size sz;
		matrix_size space;
	};

	template<typename T>
	class observer
	{
	public:
		virtual ~observer() {}
		virtual void handle_event(const T& obj){}
	};

	template<typename T>
	class observable_object
	{
	public:
		virtual ~observable_object() {}

		void add_observer(observer<T>* obs);
		void delete_observer(observer<T>* obs);

	protected:
		void notify();

	private:
		std::vector<observer<T>*> observers;
	};

	template<typename T>
	void observable_object<T>::add_observer(observer<T>* obs)
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
	void observable_object<T>::delete_observer(observer<T>* obs)
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
	void observable_object<T>::notify()
	{
		for (auto observer_ : observers)
		{
			observer_->handle_event(*(dynamic_cast<T*>(this)));
		}
	}

	template<class T, class A = std::allocator<T>>
	class matrix : private MatrixBase<T, A>, private observable_object<matrix<T, A>>
	{
		using MBase = MatrixBase<T, A>;

	public:
		class row;
		class column;

		using allocator_type = A;
		using value_type = T;

		matrix() : MBase() {}
		matrix(const matrix_size& dim) : MBase(dim.row, dim.col) {}
		matrix(const allocator_type& al) : MBase(al) {}
		matrix(Index x, Index y, const allocator_type& al = allocator_type())
			: MBase(x, y, al) 
		{ initialize(T()); }
		matrix(Index x, Index y, const value_type& val, const allocator_type& al = allocator_type())
			: MBase(x, y, al)
		{ initialize(val); }

		row at_row(Index x)
		{
			//boundary_check(x, this->sz.row);
			//this->notify();
			return row(this->elem[x], x, this->sz.col);
		}

		const row at_row(Index x) const
		{
			//boundary_check(x, this->sz.row);
			//this->notify();
			return row(this->elem[x], x, this->sz.col);
		}

	private:
		void boundary_check(Index x, Index n)
		{
			if (x < 0 || x >= n)
				throw std::out_of_range{ "index is out of range of matrix" };
		}

		void initialize(const T& val)
		{
			for (Index i = 0; i < this->sz.row; ++i)
				for (Index j = 0; j < this->sz.col; ++j)
					this->elem[i][j] = val;
		}
	};

	template<class T, class A>
	class matrix<T, A>::row : public observer<typename matrix<T, A>::row>
	{
		friend class matrix<T, A>;
	public:
		T& at(Index x)
		{
			//bound_check(x, sz);
			return elems[x];
		}

		const T& at(Index x) const
		{
			//bound_check(x, sz);
			return elems[x];
		}

		row() {}
		row(T* p, Index indx, Index n) : elems{ p }, sz { n }, index{ indx } {}

		row(const row& other) = default;
		row& operator=(const row& other) = default;

		row(row&& other) = default;
		row& operator=(row&& other) = default;

		virtual void handle_event(const matrix<T, A>& parent)
		{
			//assert(index < 0 || index >= parent.sz.row);

			std::cout << "synch states of a row and of a matrix..." << std::endl;

			//sz = parent.sz.col;
			//elems = parent.elem[index];
		}

	private:
		void bound_check(Index x, Index n)
		{
			if (x < 0 || x >= n)
				throw std::out_of_range{ "index is out of range of matrix::row" };
		}

		T* elems{};
		Index index{};
		Index sz{};
	};
}

#endif // MATRIX_HPP