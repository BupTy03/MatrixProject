#pragma once
#ifndef MATRIX_HPP
#define MATRIX_HPP

#include<memory>
#include<scoped_allocator>
#include<stdexcept>
#include<cassert>
#include<initializer_list>

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

		MatrixBase(const allocator_type& al_) : alloc{ row_allocator(), al_} {}
		MatrixBase(Index x, Index y, const allocator_type& al_)
			: sz(x, y), space(x, y), alloc(row_allocator(), al_)
		{
			elem = allocate_rows(x);
			for (Index i = 0; i < x; ++i)
				elem[i] = allocate_columns(y);
		}

		~MatrixBase()
		{
			for (Index i = 0; i < sz.row; ++i)
				deallocate_columns(elem[i], sz.col);

			deallocate_rows(elem, sz.row);
		}

		allocator_type get_allocator() { return alloc.inner_allocator(); }

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

		std::scoped_allocator_adaptor<row_allocator, allocator_type> alloc;
		T** elem{};
		matrix_size sz;
		matrix_size space;
	};

	template<class T, class A = std::allocator<T>>
	class matrix : private MatrixBase<T, A>
	{
		using MBase = MatrixBase<T, A>;

	public:
		class row;
		class column;

		using allocator_type = A;
		using value_type = T;

		matrix() : MBase() {}
		matrix(const matrix_size& dim, const allocator_type& al = allocator_type()) : MBase(dim.row, dim.col, al) {}
		matrix(const allocator_type& al) : MBase(al) {}
		matrix(Index x, Index y, const allocator_type& al = allocator_type())
			: MBase(x, y, al) 
		{ initialize(T()); }
		matrix(Index x, Index y, const value_type& val, const allocator_type& al = allocator_type())
			: MBase(x, y, al)
		{ initialize(val); }

		matrix_size size() const { return this->sz; }

		T** data() { return this->elem; }
		const T* const* data() const { return this->elem; }

		row at_row(Index x)
		{
			range_check(x, this->sz.row);
			return row(this->elem[x], x, this->sz.col);
		}

		const row at_row(Index x) const
		{
			range_check(x, this->sz.row);
			return row(this->elem[x], x, this->sz.col);
		}

		row operator[](Index x) { return at_row(x); }
		const row operator[](Index x) const { return at_row(x); }

		friend std::ostream& operator<<(std::ostream& os, const matrix& mtx)
		{
			auto size_mtx = mtx.size();
			for (Index i = 0; i < size_mtx.row; ++i)
			{
				os << "| ";
				for (Index j = 0; j < size_mtx.col; ++j)
					os << (mtx.at_row(i)).at(j) << " ";

				os << "|" << std::endl;
			}			
			
			return os;
		}

	private:
		void range_check(Index x, Index n) const
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
	class matrix<T, A>::row
	{
		friend class matrix<T, A>;
	public:
		Index size() const { return sz; }

		T& at(Index x)
		{
			range_check(x, sz);
			return elems[x];
		}

		const T& at(Index x) const
		{
			range_check(x, sz);
			return elems[x];
		}

		T& operator[](Index x) { return at(x); }
		const T& operator[](Index x) const { return at(x); }

	private:
		row() = delete;
		row(T* p, Index indx, Index n) 
			: elems{ p }, sz { n }, index{ indx } {}

		row(const row& other) = default;
		row& operator=(const row& other) = default;

		row(row&& other)
		{
			if (*this == other)
				return;

			this->elems = other.elems;
			this->sz = other.sz;
			this->index = other.index;
		}
	public:
		row operator=(row&& other)
		{
			if (this == &other)
				return *this;

			if (this->sz != other.sz)
				throw std::length_error{ "matrix rows should be equal by length" };

			for (Index i = 0; i < sz; ++i) this->elems[i] = other.elems[i];

			return *this;
		}

		template<class Container>
		row operator=(const Container& cont)
		{
			this->assign(std::begin(cont), std::end(cont));
			return *this;
		}

		row operator=(std::initializer_list<T> lst)
		{
			this->assign(std::begin(lst), std::end(lst));
			return *this;
		}

		T* data() { return elems; }
		const T* data() const { return elems; }

		friend std::ostream& operator<<(std::ostream& os, const matrix::row& r)
		{
			auto size_row = r.size();
			os << "[ ";
			for (Index i = 0; i < size_row; ++i)
				os << r.at(i) << " ";

			os << "]" << std::endl;

			return os;
		}

	private:
		void range_check(Index x, Index n) const
		{
			if (x < 0 || x >= n)
				throw std::out_of_range{ "index is out of range of matrix::row" };
		}

		template<class It>
		void assign(It first_, It last_)
		{
			if(std::distance(first_, last_) != sz)
				throw std::length_error{ "matrix row should be equal to this range by length" };

			for (Index i = 0; first_ != last_; ++first_, ++i) this->elems[i] = *first_;
		}

		T* elems{};
		Index index{};
		Index sz{};
	};

}

#endif // MATRIX_HPP