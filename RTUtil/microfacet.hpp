/*
    Cornell CS5625
    Microfacet model reference interface

    This file originates from Nori, a simple educational ray tracer

    Copyright (c) 2012-2020 by Wenzel Jakob and Steve Marschner.

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#define _USE_MATH_DEFINES
#include <glm/glm.hpp>
#include <cmath>

#include "common.hpp"

namespace nori {

/**
 * \brief Convenience data structure used to pass multiple
 * parameters to the evaluation and sampling routines in \ref BSDF
 */
struct RTUTIL_EXPORT BSDFQueryRecord {
    /// Incident direction (in the local frame)
    glm::vec3 wi;

    /// Outgoing direction (in the local frame)
    glm::vec3 wo;

    /// Relative refractive index in the sampled direction
    float eta;

    /// Create a new record for sampling the BSDF
    inline BSDFQueryRecord(const glm::vec3 &wi)
        : wi(wi) { }

    /// Create a new record for querying the BSDF
    inline BSDFQueryRecord(const glm::vec3 &wi, const glm::vec3 &wo) 
        : wi(wi), wo(wo) { }
};

/**
 * \brief Superclass of all bidirectional scattering distribution functions
 */
class RTUTIL_EXPORT BSDF {
public:
    virtual ~BSDF() {}

    /**
     * \brief Sample the BSDF and return the importance weight (i.e. the
     * value of the BSDF * cos(theta_o) divided by the probability density 
     * of the sample with respect to solid angles). 
     *
     * \param bRec    A BSDF query record
     * \param sample  A uniformly distributed sample on \f$[0,1]^2\f$
     *
     * \return The BSDF value divided by the probability density of the sample
     *         sample. The returned value also includes the cosine
     *         foreshortening factor associated with the outgoing direction,
     *         when this is appropriate. A zero value means that sampling
     *         failed.
     */
    virtual glm::vec3 sample(BSDFQueryRecord &bRec, const glm::vec2 &sample) const = 0;

    /**
     * \brief Evaluate the BSDF for a pair of directions and measure
     * specified in \code bRec
     * 
     * \param bRec
     *     A record with detailed information on the BSDF query
     * \return
     *     The BSDF value, evaluated for each color channel
     */
    virtual glm::vec3 eval(const BSDFQueryRecord &bRec) const = 0;

    /**
     * \brief Compute the probability of sampling \c bRec.wo
     * (conditioned on \c bRec.wi).
     *
     * This method provides access to the probability density that
     * is realized by the \ref sample() method.
     *
     * \param bRec
     *     A record with detailed information on the BSDF query
     *
     * \return
     *     A probability/density value expressed with respect
     *     to the specified measure
     */

    virtual float pdf(const BSDFQueryRecord &bRec) const = 0;

    /**
     * \brief Return the diffuse reflectance of this surface.
     *
     * This is the a reflectance that leads to a good approximation
     * of the diffuse part of this BSDF as a lambertian reflector.
     *
     * \return
     *     The reflectance as an RGB color.
     */
    virtual glm::vec3 diffuseReflectance() const = 0;

    // Write to a stream in a format suitable for debugging output
    virtual std::ostream &debug_write(std::ostream &) const = 0;
};


inline std::ostream &operator <<(std::ostream &ost, const BSDF &b) {
    return b.debug_write(ost);
}



class RTUTIL_EXPORT Microfacet: public BSDF {
public:
    Microfacet(float alpha, float intIOR, float extIOR, const glm::vec3 R)
    : m_alpha(alpha), m_intIOR(intIOR), m_extIOR(extIOR), m_R(R) {
    }

    virtual ~Microfacet() {}

    /// Evaluate the BRDF for the given pair of directions
    glm::vec3 eval(const BSDFQueryRecord &bRec) const override;

    /// Sample the BRDF
    glm::vec3 sample(BSDFQueryRecord &bRec, const glm::vec2 &_sample) const override;

    /// Evaluate the sampling density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const override;

    /// Return the diffuse reflectance
    glm::vec3 diffuseReflectance() const override;

    std::ostream &debug_write(std::ostream &) const override;

    float alpha() const { return m_alpha; }
    float eta() const { return m_intIOR / m_extIOR; }
    float k_s() const { return m_ks; }

private:

    float evalBeckmann(const glm::vec3 &m) const;
    glm::vec3 sampleBeckmann(const glm::vec2 &sample) const;
    float smithBeckmannG1(const glm::vec3 &v, const glm::vec3 &m) const;

    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks = 1.0f;
    glm::vec3 m_R;
};


}