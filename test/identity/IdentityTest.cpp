#define BOOST_TEST_MODULE Test Quintet Identity

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(HelloWorld) {
    BOOST_TEST_MESSAGE("IdentityTest: Hello World!");
#ifdef IDENTITY_TEST
    BOOST_REQUIRE(true);
#else
    BOOST_REQUIRE(false);
#endif
}