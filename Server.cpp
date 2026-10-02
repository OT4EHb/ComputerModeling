//система из 3 серверов параллельность с отдельными очередями, fifo
//общий пуасоновский поток поступает с заданной интенсивностью,
//экспоненциально распределенное время
//какой из вариантов распределения потока серверами лучше: циклический или
//случайный с равными вероятностями (один из трех)
//среднее время ожидания заданий в очереди
//среднеквадратичное отклонение времени ожидания

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <queue>
#include <numeric>

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

class Puasson {
	double T{};
	std::shared_ptr<std::mt19937> mt;
	std::exponential_distribution<double> gen;
public:
	Puasson(std::shared_ptr<std::mt19937> &mt) :mt(mt), gen(1) {

	}
	double next() {
		T += gen(*mt.get());
		return T;
	}
	double get() const{
		return T;
	}
};

struct Task {
	double in;
	double start;
};

class Server {
	std::queue<Task> que;
	Generator g;
	double current{};
public:
	Server(std::shared_ptr<std::mt19937> &mt, double est, double eend) :g(mt) {
		g.set_interval(est, eend);
	}
	void add(double systemtime) {
		que.emplace(systemtime, -1);
	}
	void iterate(double systemtime, std::vector<double> &times) {
		while (current < systemtime) {
			if (que.empty()) return;
			Task &t = que.front();
			if (current < t.in) current = t.in;
			if (t.start == -1) {
				t.start = current;
				times.push_back(current - t.in);
			}
			current += g();
			que.pop();
		}
	}
};

static double mean(const std::vector<double> &v) {
	return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
}

static double stddev(const std::vector<double> &v) {
	if (v.size() < 2) return 0.0;
	double m = mean(v);
	double sq = 0.0;
	for (double x : v) {
		double d = x - m;
		sq += d * d;
	}
	return std::sqrt(sq / (v.size() - 1));
}

int main() {
	auto mt = std::make_shared<std::mt19937>(
		std::chrono::steady_clock::now()
		.time_since_epoch().count()
	);
	constexpr size_t N = 3;
	double estart, eend;
	std::cout << "Interval (2 value): ";
	std::cin >> estart >> eend;
	Puasson stream{mt};
	std::vector<Server> servera;
	for (size_t i{}; i < 2 * N; ++i) {
		servera.emplace_back(std::ref(mt), estart, eend);
	}
	std::uniform_int_distribution<size_t> indexG(N, 2 * N - 1);

	std::vector<double>times1;
	std::vector<double>times2;
	double systemtime{};
	size_t index{};
	while (systemtime < 200000.) {
		systemtime = stream.next();
		Server &server = servera[index];
		index = (index + 1) % N;
		server.add(systemtime);
		server.iterate(systemtime, times1);

		Server &server2 = servera[indexG(*mt.get())];
		server2.add(systemtime);
		server2.iterate(systemtime, times2);
	}
	for (size_t i{}; i <2* N; ++i) {
		servera[i].iterate(std::numeric_limits<double>::max(), times1);
	}
	auto m1 = mean(times1), m2 = mean(times2), 
		sd1 = stddev(times1), sd2 = stddev(times2);
	std::cout << m1 << " " << sd1 << "\n";
	std::cout << m2 <<" "<<sd2<< "\n\n";
	std::cout << m1 - m2 << " " << sd1 - sd2;

	return 0;
}