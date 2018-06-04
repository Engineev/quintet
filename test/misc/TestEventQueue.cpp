#include <boost/test/unit_test.hpp>
#include "misc/EventQueue.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread_only.hpp>
#include <boost/chrono.hpp>

#include "misc/Rand.h"

BOOST_AUTO_TEST_SUITE(Misc)
BOOST_AUTO_TEST_SUITE(EventQueue)

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Misc::EventQueue::Basic");
  quintet::EventQueue q;

  boost::mutex m;
  for (int i = 0; i < 10; ++i) {
    q.addEvent([&m] {
      BOOST_REQUIRE(m.try_lock());
      boost::this_thread::sleep_for(
          boost::chrono::milliseconds(quintet::intRand(0, 5)));
      m.unlock();
    });
  }

  q.wait();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()