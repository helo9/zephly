#include <tuple>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "attitude_estimation.hpp"

using namespace std;

namespace py = pybind11;

using Quaterniond = AttitudeEstimation::Quaternion<double>;
using Vector3d = AttitudeEstimation::Vector<double, 3>;

py::array_t<double> propagate_attitude(py::array_t<double> q, py::array_t<double> w, double delta_t) {
    py::buffer_info q_ = q.request(), w_ = w.request();

    if (q_.ndim != 1 || q_.size != 4) {
        throw std::runtime_error("q has wrong shape");
    }

    if (w_.ndim != 1 || w_.size != 3) {
        throw std::runtime_error("w has wrong shape");
    }

    double *q_ptr = static_cast<double*>(q_.ptr);
    double *w_ptr = static_cast<double*>(w_.ptr);

    Quaterniond Q {q_ptr[0], q_ptr[1], q_ptr[2], q_ptr[3]};
    Vector3d W {w_ptr[0], w_ptr[1], w_ptr[2]};

    AttitudeEstimation::propagate_attitude<double>(Q, W, delta_t);

    auto q_new = py::array_t<double>(4);
    py::buffer_info q_new_ = q_new.request();
    
    double *q_new_ptr = static_cast<double *>(q_new_.ptr);

    q_new_ptr[0] = Q.w;
    q_new_ptr[1] = Q.x;
    q_new_ptr[2] = Q.y;
    q_new_ptr[3] = Q.z;

    return q_new;
}

PYBIND11_MODULE(pyahrs, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("propagate_attitude", &propagate_attitude, "blabla");
}
