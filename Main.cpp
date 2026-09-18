#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <random>

class Generator {
	std::shared_ptr<std::mt19937> mt;
	std::uniform_real_distribution<double> gen;
public:
	Generator(std::shared_ptr<std::mt19937> &mt) :mt(mt) {}
	void set_interval(double min, double max) {
		gen = std::uniform_real_distribution<double>(min, max);
	}
	double operator()() {
		return gen(*mt.get());
	}
};

enum class State {
	IDLE,
	PROCESSING
};

int main() {
	unsigned n;
	std::cout << "N = ";
	std::cin >> n;

	auto mt = std::make_shared<std::mt19937>(
		std::chrono::steady_clock::now()
		.time_since_epoch().count()
	);
	auto tau = Generator{mt};
	tau.set_interval(1., 5.);
	auto sigma = Generator{mt};
	sigma.set_interval(2., 7.);

	double systemtime{},
		T1{tau()},
		T2{T1 + sigma()};
	unsigned queue = 0;
	State state{State::IDLE};

	for (unsigned i{};;) {
		std::cout
			<< (state == State::IDLE ? "IDLE" : "PROCESSING") << " "
			<< "time: " << systemtime << " "
			<< "events: " << i << " "
			<< "queue: " << queue << " "
			<< '\n';
		bool TYPE = T1 <= T2;
		systemtime = TYPE ? T1 : T2;
		//IN
		if (TYPE) {
			if (i < n) {
				if (state == State::IDLE) {
					state = State::PROCESSING;
				}
				else {
					++queue;
				}
			T1 += tau();
			}
			else {
				T1 = std::numeric_limits<double>::max();
			}
		}
		//OUT
		else {
			if (queue == 0) {
				state = State::IDLE;
				T2 = T1;
				if (i >= n) break;
			}
			else {
				++i;
				--queue;
				T2 += sigma();
			}
		}
	}
	return 0;
}