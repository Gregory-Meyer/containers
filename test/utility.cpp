#include <catch.hpp>

#include "utility.hpp"

using namespace gregjm::containers;

TEST_CASE("is_power_of_2", "[utility.hpp]") {
    CHECK(is_power_of_2(1ull));
    CHECK(is_power_of_2(2ull));
    CHECK(is_power_of_2(4ull));
    CHECK(is_power_of_2(8ull));

    CHECK(!is_power_of_2(0ull));
    CHECK(!is_power_of_2(15ull));
    CHECK(!is_power_of_2(17ull));
    CHECK(!is_power_of_2(7ull));
}

TEST_CASE("next_power_of_2", "[utility.hpp]") {
    CHECK(next_power_of_2(1ull) == 1ull);
    CHECK(next_power_of_2(2ull) == 2ull);
    CHECK(next_power_of_2(4ull) == 4ull);
    CHECK(next_power_of_2(8ull) == 8ull);

    CHECK(next_power_of_2(15ull) == 16ull);
    CHECK(next_power_of_2(17ull) == 32ull);
    CHECK(next_power_of_2(7ull) == 8ull);
    CHECK(next_power_of_2(9ull) == 16ull);
    CHECK(next_power_of_2(std::numeric_limits<unsigned long long>::max() - 1)
          == std::numeric_limits<unsigned long long>::max());
}
