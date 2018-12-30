#ifndef SIMPLE_TIMER_HPP
#define SIMPLE_TIMER_HPP

#include<iostream>
#include<type_traits>
#include<chrono>

namespace my
{
	// Simple Timer class
	template<typename T, typename = typename std::enable_if<
		std::is_same<T, std::chrono::hours>::value ||
		std::is_same<T, std::chrono::minutes>::value ||
		std::is_same<T, std::chrono::seconds>::value ||
		std::is_same<T, std::chrono::milliseconds>::value ||
		std::is_same<T, std::chrono::microseconds>::value ||
		std::is_same<T, std::chrono::nanoseconds>::value
	>::type>
		class SimpleTimer
	{
	private:
		std::chrono::steady_clock::time_point begin_time;
		std::chrono::steady_clock::time_point end_time;
		std::ostream& os;
		bool log_when_destr;
		bool stopped;
		const char* prefix;
		const char* postfix;

	public:
		SimpleTimer()
			: begin_time(std::chrono::steady_clock::now()),
			os(std::cout),
			log_when_destr(false),
			stopped(false),
			prefix("Elapsed time: "),
			postfix((std::is_same<T, std::chrono::hours>::value) ? " hours." :
			(std::is_same<T, std::chrono::minutes>::value) ? " minutes." :
				(std::is_same<T, std::chrono::seconds>::value) ? " seconds." :
				(std::is_same<T, std::chrono::milliseconds>::value) ? " milliseconds." :
				(std::is_same<T, std::chrono::microseconds>::value) ? " microseconds." :
				(std::is_same<T, std::chrono::nanoseconds>::value) ? " nanoseconds." : "."
			)
		{}

		SimpleTimer(std::ostream& os, bool lwd = false, bool stpd = false,
			const char* pref = "Elapsed time: ",
			const char* pstf =
			(std::is_same<T, std::chrono::hours>::value) ? " hours." :
			(std::is_same<T, std::chrono::minutes>::value) ? " minutes." :
			(std::is_same<T, std::chrono::seconds>::value) ? " seconds." :
			(std::is_same<T, std::chrono::milliseconds>::value) ? " milliseconds." :
			(std::is_same<T, std::chrono::microseconds>::value) ? " microseconds." :
			(std::is_same<T, std::chrono::nanoseconds>::value) ? " nanoseconds." : "."
		) : begin_time(std::chrono::steady_clock::now()),
			os(os),
			log_when_destr(lwd),
			stopped(stpd),
			prefix(pref),
			postfix(pstf) {}

		T elapsed_time()
		{
			if (!stopped)
			{
				end_time = std::chrono::steady_clock::now();
			}
			return std::chrono::duration_cast<T>(end_time - begin_time);
		}

		void start()
		{
			stopped = false;
			begin_time = std::chrono::steady_clock::now();
		}

		void stop()
		{
			end_time = std::chrono::steady_clock::now();
			stopped = true;
		}

		void log_curr_time() { os << prefix << (elapsed_time()).count() << postfix << std::endl; }

		~SimpleTimer()
		{
			if (log_when_destr)
			{
				log_curr_time();
			}
		}
	};
}

#endif // !SIMPLE_TIMER_HPP