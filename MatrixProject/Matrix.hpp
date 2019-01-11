#pragma once
#ifndef MATRIX_HPP
#define MATRIX_HPP

#include<type_traits>
#include<memory>
#include<scoped_allocator>
#include<stdexcept>
#include<initializer_list>
#include<algorithm>
#include<iterator>

namespace my
{
	template<class InputIt, class ForwardIt>
	ForwardIt my_uninitialized_move(InputIt first, InputIt last, ForwardIt d_first)
	{
		typedef typename std::iterator_traits<ForwardIt>::value_type ValueType;
		ForwardIt current = d_first;
		for (; first != last; ++first, (void) ++current) {
			::new (static_cast<void*>(std::addressof(*current))) ValueType(std::move(*first));
		}
		return current;
	}

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
		using allocator_type = typename std::allocator_traits<A>::template rebind_alloc<T>;
		using row_allocator = typename std::allocator_traits<A>::template rebind_alloc<T*>;

		MatrixBase() {}
		explicit MatrixBase(const allocator_type& al_) : alloc{ row_allocator(), al_} {}

		explicit MatrixBase(const allocator_type& al_, matrix_size size_mtx)
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

		void swap(MatrixBase& right)
		{
			std::swap(alloc, right.alloc);
			std::swap(elem, right.elem);
			std::swap(sz, right.sz);
			std::swap(space, right.space);
		}

	protected:
		struct RowDeleter
		{
			explicit RowDeleter(const allocator_type& allocator_, Index size_) : size_{ size_ }, alloc_{ allocator_ }{}
			void operator()(T* p)
			{
				if (p == nullptr) return;
				alloc_.deallocate(p, size_);
			}
		private:
			Index size_{};
			allocator_type alloc_;
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
		std::unique_ptr<T*[], MtxDeleter> make_matrix(Index count_r)
		{
			return std::unique_ptr<T*[], MtxDeleter>(
				this->alloc.allocate(count_r),
				MtxDeleter(this->alloc, count_r)
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

		class MatrixIterator;
		class ConstMatrixIterator;

		using allocator_type = A;
		using size_type = matrix_size;
		using value_type = T;
		using iterator = MatrixIterator;
		using const_iterator = ConstMatrixIterator;

		matrix() : MBase() {}
		explicit matrix(size_type dim, const allocator_type& al = allocator_type()) : MBase(al, dim)
		{ initialize(); }
		explicit matrix(const allocator_type& al) : MBase(al) {}
		explicit matrix(Index x, Index y, const allocator_type& al = allocator_type())
			: MBase(al, size_type(x, y))
		{ initialize(); }
		explicit matrix(Index x, Index y, const value_type& val, const allocator_type& al = allocator_type())
			: MBase(al, size_type(x, y))
		{ initialize(val); }
		explicit matrix(size_type sz, const value_type& val, const allocator_type& al = allocator_type())
			: MBase(al, sz)
		{ initialize(val); }

		matrix(const matrix& other) : MBase(other.alloc, other.sz)
		{
			for (Index i = 0; i < other.sz.row; ++i)
				std::uninitialized_copy(other.elem[i], other.elem[i] + other.sz.col, this->elem[i]);
		}
		matrix& operator=(const matrix& other)
		{
			if (this == &other) return *this;

			for (Index i = 0; i < this->sz.row; ++i)
				for(Index j = 0; j < this->sz.col; ++j)
					(this->alloc.inner_allocator()).destroy(&(this->elem[i][j]));

			if (this->space.row < other.sz.row || this->space.col >= other.sz.col)
				reserve(other.sz);

			for (Index i = 0; i < other.sz.row; ++i)
				std::uninitialized_copy(other.elem[i], other.elem[i] + other.sz.col, this->elem[i]);

			this->sz = other.sz;

			return *this;
		}
		matrix(matrix&& other) { this->swap(other); }
		matrix& operator=(matrix&& other)
		{
			if (this == &other) return *this;

			this->swap(other);

			MBase tmp;
			tmp.swap(other);
			return *this;
		}

		~matrix() 
		{
			for (Index i = 0; i < this->sz.row; ++i)
				for (Index j = 0; j < this->sz.col; ++j)
					(this->alloc.inner_allocator()).destroy(&(this->elem[i][j]));
		}

		size_type size() const { return this->sz; }
		size_type capacity() const { return this->space; }

		T** data() { return this->elem; }
		const T* const* data() const { return this->elem; }

		iterator begin() { return iterator(this->elem, this->sz.col); }
		iterator end() { return iterator(this->elem + this->sz.row, this->sz.col); }

		const_iterator begin() const { return const_iterator(this->elem, this->sz.col); }
		const_iterator end() const { return const_iterator(this->elem + this->sz.row, this->sz.col); }

		const_iterator cbegin() const { return const_iterator(this->elem, this->sz.col); }
		const_iterator cend() const { return const_iterator(this->elem + this->sz.row, this->sz.col); }

		std::reverse_iterator<iterator> rbegin() { return std::reverse_iterator<iterator>(iterator(this->elem + this->sz.row - 1, this->sz.col)); }
		std::reverse_iterator<iterator> rend() { return std::reverse_iterator<iterator>(iterator(this->elem - 1, this->sz.col)); }

		std::reverse_iterator<const_iterator> rbegin() const { return std::reverse_iterator<const_iterator>(const_iterator(this->elem + this->sz.row - 1, this->sz.col)); }
		std::reverse_iterator<const_iterator> rend() const { return std::reverse_iterator<const_iterator>(const_iterator(this->elem - 1, this->sz.col)); }

		std::reverse_iterator<const_iterator> rcbegin() const { return std::reverse_iterator<const_iterator>(const_iterator(this->elem + this->sz.row - 1, this->sz.col)); }
		std::reverse_iterator<const_iterator> rcend() const { return std::reverse_iterator<const_iterator>(const_iterator(this->elem - 1, this->sz.col)); }

		Row row(Index x)
		{
			range_check(x, this->sz.row);
			return Row(this->elem[x], this->sz.col);
		}
		const Row row(Index x) const
		{
			range_check(x, this->sz.row);
			return Row(this->elem[x], this->sz.col);
		}

		Row operator[](Index x) { return row(x); }
		const Row operator[](Index x) const { return row(x); }

		T& at(Index x, Index y) 
		{
			range_check(x, this->sz.row);
			range_check(y, this->sz.col);
			return this->elem[x][y]; 
		}
		const T& at(Index x, Index y) const 
		{
			range_check(x, this->sz.row);
			range_check(y, this->sz.col);
			return this->elem[x][y]; 
		}

		void swap(matrix& other) { dynamic_cast<MBase*>(this)->swap(dynamic_cast<MBase&>(other)); }

		void swap_rows(Index r1, Index r2)
		{
			range_check(r1, this->sz.row);
			range_check(r2, this->sz.row);
			std::swap(this->elem[r1], this->elem[r2]);
		}
		void swap_cols(Index c1, Index c2)
		{
			range_check(c1, this->sz.col);
			range_check(c2, this->sz.col);
			for (Index i = 0; i < this->sz.row; ++i)
				std::swap(this->elem[i][c1], this->elem[i][c2]);
		}

		void reserve(size_type newalloc)
		{
			if (newalloc.row > this->space.row && newalloc.row == 0)
			{
				this->elem = this->alloc.allocate(newalloc.row);
				this->space.row = newalloc.row;

				Index new_all_col = std::max(newalloc.col, this->space.col);
				if (new_all_col > 0)
				{
					for (Index i = 0; i < this->space.row; ++i)
						(this->alloc.inner_allocator()).allocate(new_all_col);
				}

				return;
			}

			if (newalloc.col > this->space.col)
			{
				if (this->space.col == 0)
				{
					for (Index i = 0; i < this->space.row; ++i)
						this->elem[i] = (this->alloc.inner_allocator()).allocate(newalloc.col);

					this->space.col = newalloc.col;
				}
				else
				{
					for (Index i = 0; i < this->sz.row; ++i)
					{
						auto new_row = this->make_row(newalloc.col);

						if (std::is_nothrow_move_constructible_v<T>)
							my_uninitialized_move(this->elem[i], &(this->elem[i][this->sz.col]), new_row.get());
						else
							std::uninitialized_copy(this->elem[i], &(this->elem[i][this->sz.col]), new_row.get());

						this->delete_row(i);
						this->elem[i] = new_row.release();

						this->space.col = newalloc.col;
					}
				}
			}

			if (newalloc.row > this->space.row)
			{
				auto new_mtx = this->make_matrix(newalloc.row);

				std::copy(this->elem, &(this->elem[this->sz.row]), new_mtx.get());

				for (Index i = this->sz.row; i < newalloc.row; ++i)
					new_mtx[i] = (this->alloc.inner_allocator()).allocate(this->space.col);

				this->delete_matrix();
				this->elem = new_mtx.release();

				this->space.row = newalloc.row;
			}
		}
		void reserve(Index newalloc_row, Index newalloc_col)
		{
			reserve(size_type(newalloc_row, newalloc_col));
		}

		void resize(size_type newsize)
		{
			reserve(newsize);
			if (newsize.col > this->sz.col)
			{
				for (Index i = 0; i < this->sz.row; ++i)
					for (Index j = this->sz.col; j < newsize.col; ++j)
						(this->alloc.inner_allocator()).construct(&(this->elem[i][j]));

				this->sz.col = newsize.col;
			}

			if (newsize.row > this->sz.row)
			{
				for (Index i = this->sz.row; i < newsize.row; ++i)
					for (Index j = 0; j < this->sz.col; ++j)
						(this->alloc.inner_allocator()).construct(&(this->elem[i][j]));

				this->sz.row = newsize.row;
			}
		}
		void resize(Index newsize_row, Index newsize_col)
		{
			resize(size_type(newsize_row, newsize_col));
		}

		template<class It>
		void add_row(It first, It last)
		{
			auto dist_ = std::distance(first, last);
			if (dist_ != this->sz.col)
				throw std::out_of_range{ "columns count is not equal to new row" };

			reserve(size_type(this->sz.row + 1, this->sz.col));

			for (Index i = 0; i < dist_; ++i, ++first)
				(this->alloc.inner_allocator()).construct(&(this->elem[this->sz.row][i]), *first);

			this->sz.row += 1;
		}
		template<class Container>
		void add_row(const Container& cont) { add_row(std::cbegin(cont), std::cend(cont)); }

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
		void initialize()
		{
			Index i = 0;
			Index j = 0;
			try {
				for (; i < this->sz.row; ++i)
					for (j = 0; j < this->sz.col; ++j)
						(this->alloc.inner_allocator()).construct(&(this->elem[i][j]));
			}
			catch (const std::exception& exc) {
				for (Index m = 0; m <= i; ++m)
				{
					Index k = (m == i) ? j : this->sz.col - 1;
					for (Index n = 0; n <= k; ++n)
						(this->alloc.inner_allocator()).destroy(&(this->elem[m][n]));
				}

				this->sz.row = 0;
				this->sz.col = 0;

				throw exc;
			}
			catch (...) {
				for (Index m = 0; m <= i; ++m)
				{
					Index k = (m == i) ? j : this->sz.col - 1;
					for (Index n = 0; n <= k; ++n)
						(this->alloc.inner_allocator()).destroy(&(this->elem[m][n]));
				}

				this->sz.row = 0;
				this->sz.col = 0;

				throw;
			}
		}
		void initialize(const T& val)
		{
			Index i = 0;
			Index j = 0;
			try {
				for (; i < this->sz.row; ++i)
					for (j = 0; j < this->sz.col; ++j)
						(this->alloc.inner_allocator()).construct(&(this->elem[i][j]), val);
			}
			catch (const std::exception& exc) {
				for (Index m = 0; m <= i; ++m)
				{
					Index k = (m == i) ? j : this->sz.col - 1;
					for (Index n = 0; n <= k; ++n)
						(this->alloc.inner_allocator()).destroy(&(this->elem[m][n]));
				}

				this->sz.row = 0;
				this->sz.col = 0;

				throw exc;
			}
			catch (...) {
				for (Index m = 0; m <= i; ++m)
				{
					Index k = (m == i) ? j : this->sz.col - 1;
					for (Index n = 0; n <= k; ++n)
						(this->alloc.inner_allocator()).destroy(&(this->elem[m][n]));
				}

				this->sz.row = 0;
				this->sz.col = 0;

				throw;
			}
		}
	};

	template<class T, class A>
	class matrix<T, A>::MatrixIterator : public std::iterator<std::input_iterator_tag, T>
	{
		friend class matrix<T, A>;
	private:
		MatrixIterator(T** p, Index num) : p{ p }, row_{ *p, num } {}

	public:
		MatrixIterator(const MatrixIterator &it) : p{ it.p }, row_{ it.row_ } {}
		MatrixIterator& operator =(const MatrixIterator&) = default;

		inline bool operator==(MatrixIterator const& other) const { return p == other.p; }
		inline bool operator!=(MatrixIterator const& other) const { return p != other.p; }
		inline matrix<T, A>::Row& operator*() { return row_; }
		inline matrix<T, A>::Row* operator->() noexcept { return &row_; }
		inline MatrixIterator& operator++()
		{
			++p;
			row_.elems = *p;
			return *this;
		}
		inline MatrixIterator& operator++(int)
		{
			auto _tmp = *this;
			this->operator++();
			return _tmp;
		}
		inline MatrixIterator& operator--()
		{
			--p;
			row_.elems = *p;
			return *this;
		}
		inline MatrixIterator& operator--(int)
		{
			auto _tmp = *this;
			this->operator--();
			return _tmp;
		}
	private:
		T** p{};
		matrix<T, A>::Row row_{};
	};

	template<class T, class A>
	class matrix<T, A>::ConstMatrixIterator : public std::iterator<std::input_iterator_tag, T, ptrdiff_t, T*, const T&>
	{
		friend class matrix<T, A>;
	private:
		ConstMatrixIterator(T** p, Index num) noexcept : p{ p }, row_{ *p, num } {}

	public:
		ConstMatrixIterator(const ConstMatrixIterator &it) noexcept : p{ it.p }, row_{ it.row_ } {}
		ConstMatrixIterator& operator =(const ConstMatrixIterator&) = default;

		inline bool operator==(ConstMatrixIterator const& other) const noexcept { return p == other.p; }
		inline bool operator!=(ConstMatrixIterator const& other) const noexcept { return p != other.p; }
		inline const matrix<T, A>::Row& operator*() const noexcept { return row_; }
		inline const matrix<T, A>::Row* operator->() const noexcept { return &row_; }
		inline ConstMatrixIterator& operator++() noexcept
		{
			++p;
			row_.elems = *p;
			return *this;
		}
		inline ConstMatrixIterator& operator++(int) noexcept
		{
			auto _tmp = *this;
			this->operator++();
			return _tmp;
		}
		inline ConstMatrixIterator& operator--()
		{
			--p;
			row_.elems = *p;
			return *this;
		}
		inline ConstMatrixIterator& operator--(int)
		{
			auto _tmp = *this;
			this->operator--();
			return _tmp;
		}
	private:
		T** p{};
		matrix<T, A>::Row row_{};
	};

	template<class T, class A>
	class matrix<T, A>::Row
	{
		friend class matrix<T, A>;
	public:
		class MatrixRowIterator;
		class ConstMatrixRowIterator;

		using iterator = MatrixRowIterator;
		using const_iterator = ConstMatrixRowIterator;

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
		explicit Row(T* p, Index n)
			: elems{ p }, sz { n } {}

		Row(const Row& other) = default;
		Row(Row&& other)
		{
			if (*this == other)
				return;

			this->elems = other.elems;
			this->sz = other.sz;
		}

	public:
		Row& operator=(const Row& other) = default;
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

		template<class It>
		void assign(It first_, It last_)
		{
			if (std::distance(first_, last_) != sz)
				throw std::length_error{ "matrix row should be equal to this range by length" };

			for (Index i = 0; first_ != last_; ++first_, ++i) this->elems[i] = *first_;
		}

		iterator begin() { return iterator(this->elems); }
		iterator end() { return iterator(this->elems + this->sz); }

		const_iterator begin() const { return const_iterator(this->elems); }
		const_iterator end() const { return const_iterator(this->elems + this->sz); }

		const_iterator cbegin() const { return const_iterator(this->elems); }
		const_iterator cend() const { return const_iterator(this->elems + this->sz); }

		std::reverse_iterator<iterator> rbegin() { return std::reverse_iterator<iterator>(iterator(this->elems + this->sz)); }
		std::reverse_iterator<iterator> rend() { return std::reverse_iterator<iterator>(iterator(this->elems)); }

		std::reverse_iterator<const_iterator> rbegin() const { return std::reverse_iterator<const_iterator>(const_iterator(this->elems + this->sz)); }
		std::reverse_iterator<const_iterator> rend() const { return std::reverse_iterator<const_iterator>(const_iterator(this->elems)); }

		std::reverse_iterator<const_iterator> rcbegin() const { return std::reverse_iterator<const_iterator>(const_iterator(this->elems + this->sz)); }
		std::reverse_iterator<const_iterator> rcend() const { return std::reverse_iterator<const_iterator>(const_iterator(this->elems)); }

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

		T* elems{};
		Index sz{};
	};

	template<class T, class A>
	class matrix<T, A>::Row::MatrixRowIterator : public std::iterator<std::input_iterator_tag, T>
	{
		friend class matrix<T, A>;
	private:
		MatrixRowIterator(T* p) noexcept : p{ p } {}

	public:
		MatrixRowIterator(const MatrixRowIterator& it) noexcept : p{ it.p } {}
		MatrixRowIterator& operator =(const MatrixRowIterator&) = default;

		inline bool operator==(MatrixRowIterator const& other) const noexcept { return p == other.p; }
		inline bool operator!=(MatrixRowIterator const& other) const noexcept { return p != other.p; }
		inline T& operator*() const noexcept { return *p; }
		inline MatrixRowIterator& operator++() noexcept
		{
			++p;
			return *this;
		}
		inline MatrixRowIterator& operator++(int) noexcept
		{
			auto _tmp = *this;
			this->operator++();
			return _tmp;
		}
		inline MatrixRowIterator& operator--() noexcept
		{
			--p;
			return *this;
		}
		inline MatrixRowIterator& operator--(int) noexcept
		{
			auto _tmp = *this;
			this->operator--();
			return _tmp;
		}
	private:
		T* p{};
	};

	template<class T, class A>
	class matrix<T, A>::Row::ConstMatrixRowIterator : public std::iterator<std::input_iterator_tag, T, ptrdiff_t, T*, const T&>
	{
		friend class matrix<T, A>;
	private:
		ConstMatrixRowIterator(T* p) noexcept : p{ p } {}

	public:
		ConstMatrixRowIterator(const ConstMatrixRowIterator &it) noexcept : p{ it.p } {}
		ConstMatrixRowIterator& operator =(const ConstMatrixRowIterator&) = default;

		inline bool operator==(ConstMatrixRowIterator const& other) const noexcept { return p == other.p; }
		inline bool operator!=(ConstMatrixRowIterator const& other) const noexcept { return p != other.p; }
		inline const T& operator*() const noexcept { return *p; }
		inline const T* operator->() const noexcept { return p; }
		inline ConstMatrixRowIterator& operator++() noexcept
		{
			++p;
			return *this;
		}
		inline ConstMatrixRowIterator& operator++(int) noexcept
		{
			auto _tmp = *this;
			this->operator++();
			return _tmp;
		}
		inline ConstMatrixRowIterator& operator--() noexcept
		{
			--p;
			return *this;
		}
		inline ConstMatrixRowIterator& operator--(int) noexcept
		{
			auto _tmp = *this;
			this->operator--();
			return _tmp;
		}
	private:
		const T* p{};
	};
}

#endif // MATRIX_HPP