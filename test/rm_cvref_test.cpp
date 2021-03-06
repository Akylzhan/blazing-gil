#include <catch2/catch.hpp>

#include <flash/core.hpp>
#include <type_traits>

TEST_CASE("remove_cv_ref_t for int", "[rm_cvref]")
{
    STATIC_REQUIRE(std::is_same_v<flash::remove_cvref_t<int>, int>);
}

TEST_CASE("remove_cv_ref_t for int&", "[rm_cvref]")
{
    STATIC_REQUIRE(std::is_same_v<flash::remove_cvref_t<int&>, int>);
}

TEST_CASE("remove_cv_ref_t for const int&", "[rm_cvref]")
{
    STATIC_REQUIRE(std::is_same_v<flash::remove_cvref_t<const int&>, int>);
}

TEST_CASE("remove_cv_ref_t for volatile int&", "[rm_cvref]")
{
    STATIC_REQUIRE(std::is_same_v<flash::remove_cvref_t<volatile int&>, int>);
}
