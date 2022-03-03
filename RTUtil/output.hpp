/*
 * Cornell CS5625
 * RTUtil library
 * 
 * Some output operators that are helpful for writing GLM types
 * to standard C++ output streams.
 *
 * Author: srm, Spring 2020
 */

#pragma once

#include <iostream>
#include <glm/glm.hpp>

namespace glm {

    /*
     * An output operator for GLM vector types, suitable for debugging output.
     */
    template<int N, class T, glm::qualifier Q>
    std::ostream &operator <<(std::ostream &ost, const glm::vec<N, T, Q> &v) {
        ost << "[ ";
        for (int i = 0; i < N; i++)
            ost << +v[i] << " ";   // unary + hack to ensure numeric output even for char
        return ost << "]";
    }

    /*
     * An output operator for GLM matrix types, suitable for debugging output.
     */
    template<int N, int M, class T, glm::qualifier Q>
    std::ostream &operator <<(std::ostream &ost, const glm::mat<N, M, T, Q> &m) {
        ost << "[ ";
        for (int i = 0; i < M; i++) {
            if (i != 0) ost << "; ";
            for (int j = 0; j < N; j++)
                ost << +m[j][i] << " ";   // unary + hack to ensure numeric output
        }
        return ost << "]";
    }

}
