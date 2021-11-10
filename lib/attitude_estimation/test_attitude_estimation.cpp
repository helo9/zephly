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

#include "attitude_estimation.hpp"

using namespace Catch;
using namespace AttitudeEstimation;

using mathtypes = std::tuple<float, double>;

TEMPLATE_LIST_TEST_CASE( "Attitude is propaged", "[ahrs]", mathtypes ) 
{
    Quaternion<TestType> q{1.0, 0.0, 0.0};
    Vector<TestType, 3> omega{0.1, 0.0, 0.0};

    propagate_attitude(q, omega, static_cast<TestType>(0.1));

    TestType norm = sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);

    REQUIRE( q.w == Approx(0.99997500).epsilon(0.001) );
    REQUIRE( q.x == Approx(0.00499988).epsilon(0.001) );
    REQUIRE( q.y == Approx(0.0).epsilon(0.001) );
    REQUIRE( q.z == Approx(0.0).epsilon(0.001) );
    REQUIRE( norm == Approx(1.0).epsilon(0.001) );
}

TEMPLATE_LIST_TEST_CASE( "Propagate multiple times", "[ahrs]", mathtypes )
{
    Quaternion<TestType> q{1.0, 0.0, 0.0};
    Vector<TestType, 3> omega{0.1, 0.0, 0.0};

    auto data = xt::load_npy<double>("examples/example_data.npy");

    auto row_num = xt::adapt(data.shape())(0);

    for (auto i = 0; i < row_num; i++) {
        omega.data[0] = data(i, 0);
        omega.data[1] = data(i, 1);
        omega.data[2] = data(i, 2);

        propagate_attitude<TestType>(q, omega, static_cast<TestType>(0.001));

        double norm = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;

        REQUIRE( norm == Approx(1.0) );
    }
}
