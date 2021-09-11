#include <iostream>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xnpy.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xaxis_slice_iterator.hpp>
#include <xtensor/xadapt.hpp>
#include <math.h>
#include <stdio.h>

#include "attitude_propagation.hpp"

using namespace Catch;
using namespace ahrs;

using mathtypes = std::tuple<float, double>;

TEMPLATE_LIST_TEST_CASE( "Attitude is propaged", "[ahrs]", mathtypes ) 
{
    Quaternion<TestType> q{1.0, 0.0, 0.0};
    Vector<TestType, 3> omega{0.1, 0.0, 0.0};

    propagate_attitude(q, omega, static_cast<TestType>(0.1));

    TestType norm = sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);

    REQUIRE( q.w == Approx(0.99997500) );
    REQUIRE( q.x == Approx(0.00499988) );
    REQUIRE( q.y == Approx(0.0) );
    REQUIRE( q.z == Approx(0.0) );
    REQUIRE( norm == Approx(1.0).margin(1e-4) );
}

TEMPLATE_LIST_TEST_CASE( "Propagate multiple times", "[ahrs]", mathtypes )
{
    Quaternion<TestType> q{1.0, 0.0, 0.0};
    Vector<TestType, 3> omega{0.1, 0.0, 0.0};

    auto data = xt::load_npy<double>("test.npy");

    auto row_num = xt::adapt(data.shape())(0);

    for (auto i = 0; i < row_num; i++) {
        omega.data[0] = data(i, 0);
        omega.data[1] = data(i, 1);
        omega.data[2] = data(i, 2);

        propagate_attitude<TestType>(q, omega, static_cast<TestType>(0.001));

        double norm = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;

        //printf("%f - %f - %f - %f -- %f\n", q.w, q.x, q.y, q.z, norm);

        REQUIRE( norm == Approx(1.0) );
    }
}
