#include <boost/test/unit_test.hpp>
#include "identity/IdentityLeader.h"

#include "IdentityTestHelper.h"

namespace utf = boost::unit_test;

//BOOST_AUTO_TEST_SUITE(Identity)
//BOOST_FIXTURE_TEST_SUITE(Leader, quintet::test::IdentityTestHelper)

//BOOST_AUTO_TEST_CASE(Basic) {
//  BOOST_TEST_MESSAGE("Test::Identity::Leader::Basic");
//  using No = quintet::ServerIdentityNo;
//  auto srv = makeServers(2);
//  srv.front()->setBeforeTransform([] (No from, No to) {
//    if (from == No::Down && to == No::Follower)
//      return No::Leader;
//    if (to == No::Down)
//      return No::Down;
//    throw ;
//  });
//  srv.front()->AsyncRun();
//  for (std::size_t i = 1, sz = srv.size(); i < sz; ++i) {
//    srv[i]->setBeforeTransform([] (No from, No to) {
//      if ((from == No::Down && to == No::Follower) || to == No::Down)
//        return to;
//      throw ;
//    });
//  }
//
//}
//
//BOOST_AUTO_TEST_SUITE_END()
//BOOST_AUTO_TEST_SUITE_END()