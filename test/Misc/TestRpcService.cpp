#include <boost/test/unit_test.hpp>
#include "RpcService.h"

#include <utility>
#include <thread>
#include <vector>
#include <chrono>
#include <random>

#include <rpc/client.h>

#include "ServerInfo.h"
#include "RaftDefs.h"

BOOST_AUTO_TEST_SUITE(Misc)
BOOST_AUTO_TEST_SUITE(RpcService)

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_CASE(Raft, *utf::disabled()) {
	BOOST_TEST_MESSAGE("Misc::RpcService::Raft");
	using namespace quintet;

	quintet::RpcService rpcSrv;
	rpcSrv.listen(8000);
	rpcSrv.bind("AppendEntries",
		[](Term term, ServerId leaderId,
			std::size_t prevLogIdx, Term prevLogTerm,
			std::vector<LogEntry> logEntries, std::size_t commitIdx)
		-> std::pair<Term, bool> { return { 0, 0 }; });
	rpcSrv.bind("RequestVote",
		[](Term term, ServerId candidateId,
			std::size_t lastLogIdx, Term lastLogTerm)
		-> std::pair<Term, bool> { return { 0, 0 }; });
	rpcSrv.async_run(4);

	const std::size_t N = 15;
	const std::int64_t TimeMs = 5000;

	std::vector<std::thread> ts(N);
	for (int i = 0; i < (int)N; ++i)
		ts[i] = std::thread([&] {
		rpc::client c("localhost", 8000);
		std::default_random_engine e{ std::random_device()() };
		std::uniform_int_distribution<int> uniform_dist(0, 50);
		auto randInt = std::bind(uniform_dist, e);
		auto start = std::chrono::high_resolution_clock::now();
		while (std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - start).count() < TimeMs) {
			std::this_thread::sleep_for(std::chrono::milliseconds(randInt()));
			if (randInt() % 2)
				c.call("AppendEntries", 0, ServerId(), 0, 0,
					std::vector<LogEntry>(), 0);
			else
				c.call("RequestVote", 0, ServerId(), 0, 0);
		}
	});

	for (auto & t : ts)
		t.join();

	BOOST_REQUIRE_NO_THROW(rpcSrv.stop());
}

BOOST_AUTO_TEST_CASE(PauseWhenCalling/*, *utf::disabled()*/) {
	BOOST_TEST_MESSAGE("Misc::RpcService::PauseWhenCalling");

	quintet::RpcService rpcSrv;
	const size_t callNum = 15;

	rpcSrv.listen(8000);
	rpcSrv.bind("TimeConsuming", [](int i) {
		std::default_random_engine e{ std::random_device()() };
		std::uniform_int_distribution<int> uniform_dist(1000, 2000);
		auto randTime = std::bind(uniform_dist, e);
		std::this_thread::sleep_for(std::chrono::milliseconds(randTime()));
		//std::cerr << "Complete " << i << '\n';
		return i;
	});
	rpcSrv.async_run(4);

	std::vector<std::unique_ptr<rpc::client>> clients;
	std::vector<std::future<RPCLIB_MSGPACK::object_handle>> futs;
	for (size_t i = 0; i < callNum; ++i) {
		clients.emplace_back(std::make_unique<rpc::client>("localhost", 8000));
	}

	for (size_t i = 0; i < callNum; ++i) {
		//std::cerr << "Call " + std::to_string(i) + "\n";
		futs.emplace_back(clients[i]->async_call("TimeConsuming", i));
	}

	auto pauseThread = std::thread([&]() {
		//std::cerr << "Pause server\n";
		rpcSrv.pause();
		//std::cerr << "Paused\n";
		//std::cerr << "Call " + std::to_string(0) + "\n";
		futs.emplace_back(clients[0]->async_call("TimeConsuming", 0));
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		//std::cerr << "Resume server\n";
		rpcSrv.resume();
		//std::cerr << "Call " + std::to_string(1) + "\n";
		futs.emplace_back(clients[1]->async_call("TimeConsuming", 1));
	});

	pauseThread.join();
	for (auto &fut : futs) {
		fut.get().as<int>();
	}
	BOOST_REQUIRE_NO_THROW(rpcSrv.stop());
}

BOOST_AUTO_TEST_CASE(StopWhenPausing, *utf::disabled()) {
	BOOST_TEST_MESSAGE("Misc::RpcService::StopWhenPausing");
	quintet::RpcService rpcSrv;
	rpcSrv.listen(8000);
	rpcSrv.bind("TimeConsuming", [](int i) {
		std::default_random_engine e{ std::random_device()() };
		std::uniform_int_distribution<int> uniform_dist(1000, 2000);
		auto randTime = std::bind(uniform_dist, e);
		std::this_thread::sleep_for(std::chrono::milliseconds(randTime()));
		std::cerr << "Run " + std::to_string(i) + '\n';
		return true;
	});
	rpcSrv.async_run(4);

	const size_t callNum = 5;
	std::vector<std::thread> callThread(callNum);
	std::vector<std::unique_ptr<rpc::client>> clients;
	for (size_t i = 0; i < callNum; ++i) {
		clients.emplace_back(std::make_unique<rpc::client>("localhost", 8000));
	}

	// TODO

}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()