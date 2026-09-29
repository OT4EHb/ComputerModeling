#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <queue>

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

struct Stream {
	double T;
	double phase;
	void next() {
		T += phase;
	}
};

static decltype(auto) operator<(const Stream &a, const Stream &b) {
	return a.T > b.T;
}

int main() {
	unsigned N;
	std::cout << "N = ";
	std::cin >> N;

	auto mt = std::make_shared<std::mt19937>(
		std::chrono::steady_clock::now()
		.time_since_epoch().count()
	);
	Generator Tgen{mt};
	Tgen.set_interval(0, 1);
	Generator phgen{mt};
	phgen.set_interval(0, 1);

	std::priority_queue<Stream>pq;
	for (size_t i{}; i < N; ++i) {
		pq.emplace(Tgen(), phgen());
	}

	double systemtime{};
	while (systemtime < 8.) {
		Stream m = pq.top();
		pq.pop();
		systemtime = m.T;
		printf_s("%.5f ", systemtime);
		m.next();
		pq.push(m);
	}

	return 0;
}