#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Utility
#include <boost/test/unit_test.hpp>
#include "../utility.h"

using namespace boost;
using namespace boost::unit_test;

int add(int i, int j)
{
    return i + j;
}

BOOST_AUTO_TEST_CASE(universeInOrder)
{
    BOOST_CHECK(add(2, 2) == 4);
}

BOOST_AUTO_TEST_CASE(stdhelper_exists_for_map)
{
    std::map<int, int> m;
    m[1] = 1;
    BOOST_CHECK_EQUAL(stdhelper::exists(m, 1), true);
    BOOST_CHECK_EQUAL(stdhelper::exists(m, 2), false);

    std::map<string, int> m2;
    string a = "key";
    m2[a] = 2;
    BOOST_CHECK_EQUAL(stdhelper::exists(m2, a), true);

    m2.erase(a);
    BOOST_CHECK_EQUAL(stdhelper::exists(m2, a), false);
}

BOOST_AUTO_TEST_CASE(stdhelper_exists_for_set)
{
    std::set<int> collection;
    collection.insert(1);
    BOOST_CHECK_EQUAL(stdhelper::exists(collection, 1), true);
    BOOST_CHECK_EQUAL(stdhelper::exists(collection, 2), false);

    std::set<string> collection2;
    string a = "key";
    collection2.insert(a);
    BOOST_CHECK_EQUAL(stdhelper::exists(collection2, a), true);
    collection2.erase(a);
    BOOST_CHECK_EQUAL(stdhelper::exists(collection2, a), false);
}