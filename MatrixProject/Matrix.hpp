#pragma once
#ifndef MATRIX_HPP
#define MATRIX_HPP

#include<memory>
#include<scoped_allocator>
#include<stdexcept>
#include<cassert>
#include<initializer_list>
#include<functional>

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

		MatrixBase(const allocator_type& al_, matrix_size size_mtx)
			: alloc{ row_allocator(), al_ }, sz{ size_mtx }, space{ size_mtx }
		{
			if (size_mtx.row < 0 || size_mtx.col < 0)
				throw std::invalid_argument{ "matrix_size arguments must be positive" };

			if (size_mtx.row == 0) return;
			elem = alloc.allocate(size_mtx.row);

			if (size_mtx.col == 0) return;
			for (Index i = 0; i < size_mtx.row; ++i)
				elem[i] = (alloc.inner_allocator()).allocate(size_mtx.col);
		}

		~MatrixBase()
		{
			if (space.col > 0)
			{
				for (Index i = 0; i < space.row; ++i)
					(alloc.inner_allocator()).deallocate(elem[i], space.col);
			}
			
			alloc.deallocate(elem, space.row);
		}

		allocator_type get_allocator() { return alloc.inner_allocator(); }

	protected:
		struct RowDeleter
		{
			explicit RowDeleter(const A& allocator_, Index size_) : size_{ size_ }, alloc_{ allocator_ }{}
			void operator()(T* p)
			{
				if (p == nullptr) return;
				alloc_.deallocate(p, size_);
			}
		private:
			Index size_{};
			A alloc_;
		};
		struct MtxDeleter
		{
			explicit MtxDeleter(const std::scoped_allocator_adaptor<row_allocator, allocator_type>& allocator_, Index size_) : size_{ size_ }, alloc_{ allocator_ }{}
			void operator()(T** p)
			{
				if (p == nullptr) return;
				alloc_.deallocate(p, size_);
			}
		private:
			Index size_{};
			std::scoped_allocator_adaptor<row_allocator, allocator_type> alloc_;
		};

		std::unique_ptr<T[], RowDeleter> make_row(Index size_r)
		{
			return std::unique_ptr<T[], RowDeleter>(
				(this->alloc.inner_allocator()).allocate(size_r),
				RowDeleter(this->alloc.inner_allocator(), size_r)
			);
		}
		void delete_row(Index r)
		{
			if (this->elem[r] == nullptr) return;
			for (Index i = 0; i < this->sz.col; ++i)
			{
				(this->alloc.inner_allocator()).destroy(&this->elem[r][i]);
			}

			(this->alloc.inner_allocator()).deallocate(this->elem[r], this->space.col);
		}

		std::unique_ptr<T*[], MtxDeleter> make_matrix(Index count_r)
		{
			return std::unique_ptr<T*[], MtxDeleter>(
				this->alloc.allocate(count_r),
				MtxDeleter(this->alloc, count_r)
			);
		}
		void delete_matrix()
		{
			this->alloc.deallocate(this->elem, this->space.row);
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
		class Row;

		using allocator_type = A;
		using size_type = matrix_size;
		using value_type = T;

		matrix() : MBase() {}
		matrix(size_type dim, const allocator_type& al = allocator_type()) : MBase(dim, al) {}
		matrix(const allocator_type& al) : MBase(al) {}
		matrix(Index x, Index y, const allocator_type& al = allocator_type())
			: MBase(al, size_type(x, y))
		{ initialize(T()); }
		matrix(Index x, Index y, const value_type& val, const allocator_type& al = allocator_type())
			: MBase(al, size_type(x, y))
		{ initialize(val); }

		size_type size() const { return this->sz; }
		size_type capacity() const { return this->space; }

		T** data() { return this->elem; }
		const T* const* data() const { return this->elem; }

		Row row(Index x)
		{
			range_check(x, this->sz.row);
			return Row(this->elem[x], x, this->sz.col);
		}
		const Row row(Index x) const
		{
			range_check(x, this->sz.row);
			return Row(this->elem[x], x, this->sz.col);
		}

		Row operator[](Index x) { return row(x); }
		const Row operator[](Index x) const { return row(x); }

		void reserve(size_type newalloc)
		{
			if (newalloc.col > this->space.col)
			{
				for (Index i = 0; i < this->sz.row; ++i)
				{
					auto new_row = this->make_row(newalloc.col);
					std::uninitialized_copy(this->elem[i], &this->elem[i][this->sz.col], new_row.get());

					this->delete_row(i);
					this->elem[i] = new_row.release();

					this->space.col = newalloc.col;
				}
			}

			if (newalloc.row > this->space.row)
			{
				auto new_mtx = this->make_matrix(newalloc.row);

				std::uninitialized_copy(this->elem, &this->elem[this->sz.row], new_mtx.get());
				for (Index i = this->sz.row; i < newalloc.row; ++i)
				{
					new_mtx[i] = (this->alloc.inner_allocator()).allocate(this->space.col);
				}

				this->delete_matrix();
				this->elem = new_mtx.release();

				this->space.row = newalloc.row;
			}
		}
		void reserve(Index newalloc_row, Index newalloc_col)
		{
			reserve(size_type(newalloc_row, newalloc_col));
		}

		void resize(size_type newsize, const T& val = T())
		{
			reserve(newsize);
			if (newsize.col > this->sz.col)
			{
				for (Index i = 0; i < this->sz.row; ++i)
					for (Index j = this->sz.col; j < newsize.col; ++j)
						this->elem[i][j] = val;

				this->sz.col = newsize.col;
			}

			if (newsize.row > this->sz.row)
			{
				for (Index i = this->sz.row; i < newsize.row; ++i)
					for (Index j = 0; j < this->sz.col; ++j)
						this->elem[i][j] = val;

				this->sz.row = newsize.row;
			}
		}
		void resize(Index newsize_row, Index newsize_col, const T& val = T())
		{
			resize(size_type(newsize_row, newsize_col), val);
		}

		friend std::ostream& operator<<(std::ostream& os, const matrix& mtx)
		{
			auto size_mtx = mtx.size();
			for (Index i = 0; i < size_mtx.row; ++i)
			{
				os << "| ";
				for (Index j = 0; j < size_mtx.col; ++j)
					os << (mtx.row(i)).at(j) << " ";

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
					this->alloc.construct(&(this->elem[i][j]), val);
		}
	};

	template<class T, class A>
	class matrix<T, A>::Row
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
		Row() = delete;
		explicit Row(T* p, Index indx, Index n)
			: elems{ p }, sz { n }, index{ indx } {}

		Row(const Row& other) = default;
		Row& operator=(const Row& other) = default;

		Row(Row&& other)
		{
			if (*this == other)
				return;

			this->elems = other.elems;
			this->sz = other.sz;
			this->index = other.index;
		}
	public:
		Row operator=(Row&& other)
		{
			if (this == &other)
				return *this;

			if (this->sz != other.sz)
				throw std::length_error{ "matrix rows should be equal by length" };

			for (Index i = 0; i < sz; ++i) this->elems[i] = other.elems[i];

			return *this;
		}
		template<class Container>
		Row operator=(const Container& cont)
		{
			this->assign(std::begin(cont), std::end(cont));
			return *this;
		}
		Row operator=(std::initializer_list<T> lst)
		{
			this->assign(std::begin(lst), std::end(lst));
			return *this;
		}

		T* data() { return elems; }
		const T* data() const { return elems; }

		friend std::ostream& operator<<(std::ostream& os, const matrix::Row& r)
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

};

#endif // MATRIX_HPP