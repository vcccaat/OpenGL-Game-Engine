/*
	Cornell CS5625
    Microfacet model reference implementation

    modified from Nori educational raytracer
    Copyright (c) 2012 by Wenzel Jakob and Steve Marschner.

	DO NOT REDISTRIBUTE
*/

#include "microfacet.hpp"
#include "frame.hpp"
#include "geomtools.hpp"
#include "output.hpp"

namespace nori {

	/* A few useful constants */
	#define INV_PI       0.31830988618379067154f
	#define INV_TWOPI    0.15915494309189533577f
	#define INV_FOURPI   0.07957747154594766788f
	#define SQRT_TWO     1.41421356237309504880f
	#define INV_SQRT_TWO 0.70710678118654752440f

	float fresnel(float cosThetaI, float extIOR, float intIOR) {
		float etaI = extIOR, etaT = intIOR;

		if (extIOR == intIOR)
			return 0.0f;

		/* Swap the indices of refraction if the interaction starts
		   at the inside of the object */
		if (cosThetaI < 0.0f) {
			std::swap(etaI, etaT);
			cosThetaI = -cosThetaI;
		}

		/* Using Snell's law, calculate the squared sine of the
		   angle between the normal and the transmitted ray */
		float eta = etaI / etaT,
			  sinThetaTSqr = eta*eta * (1-cosThetaI*cosThetaI);

		if (sinThetaTSqr > 1.0f)
			return 1.0f;  /* Total internal reflection! */

		float cosThetaT = std::sqrt(1.0f - sinThetaTSqr);

		float Rs = (etaI * cosThetaI - etaT * cosThetaT)
		         / (etaI * cosThetaI + etaT * cosThetaT);
		float Rp = (etaT * cosThetaI - etaI * cosThetaT)
		         / (etaT * cosThetaI + etaI * cosThetaT);

		return (Rs * Rs + Rp * Rp) / 2.0f;
	}

	/// Evaluate the microfacet normal distribution D
	float Microfacet::evalBeckmann(const glm::vec3 &m) const {
		float temp = Frame::tanTheta(m) / m_alpha,
			  ct = Frame::cosTheta(m), ct2 = ct*ct;

		return std::exp(-temp*temp) 
			/ (M_PI * m_alpha * m_alpha * ct2 * ct2);
	}

	/// Sample D*cos(thetaM)
	glm::vec3 Microfacet::sampleBeckmann(const glm::vec2 &sample) const {
		float sinPhi, cosPhi;
		math::sincos(2.0f * M_PI * sample.x, &sinPhi, &cosPhi);

		float tanThetaMSqr = -m_alpha*m_alpha * std::log(1.0f - sample.y);
		float cosThetaM = 1.0f / std::sqrt(1+tanThetaMSqr);
		float sinThetaM = std::sqrt(std::max(0.0f, 1.0f - cosThetaM*cosThetaM));

		return glm::vec3(sinThetaM * cosPhi, sinThetaM * sinPhi, cosThetaM);
	}

	/// Evaluate Smith's shadowing-masking function G1 
	float Microfacet::smithBeckmannG1(const glm::vec3 &v, const glm::vec3 &m) const {
		float tanTheta = Frame::tanTheta(v);

		/* Perpendicular incidence -- no shadowing/masking */
		if (tanTheta == 0.0f)
			return 1.0f;

		/* Can't see the back side from the front and vice versa */
		if (glm::dot(m, v) * Frame::cosTheta(v) <= 0)
			return 0.0f;

		float a = 1.0f / (m_alpha * tanTheta);
		if (a >= 1.6f)
			return 1.0f;
		float a2 = a * a;

		/* Use a fast and accurate (<0.35% rel. error) rational
		   approximation to the shadowing-masking function */
		return (3.535f * a + 2.181f * a2) 
		     / (1.0f + 2.276f * a + 2.577f * a2);
	}

	/// Evaluate the BRDF for the given pair of directions
	glm::vec3 Microfacet::eval(const BSDFQueryRecord &bRec) const {
		/* Return zero queried for illumination on the backside */
		if (Frame::cosTheta(bRec.wi) <= 0
			|| Frame::cosTheta(bRec.wo) <= 0)
			return glm::vec3(0, 0, 0);

		/* Compute the half direction vector */
		glm::vec3 H = glm::normalize(bRec.wo+bRec.wi);

		/* Evaluate the microsurface normal distribution */
		float D = evalBeckmann(H);

		/* Fresnel factor */
		float F = fresnel(glm::dot(H, bRec.wi), m_extIOR, m_intIOR);

		/* Smith's shadow-masking function */
		float G = smithBeckmannG1(bRec.wi, H) * 
		          smithBeckmannG1(bRec.wo, H);

		/* Combine everything to obtain the specular reflectance */
		float spec = m_ks * F * D * G / 
			(4.0f * Frame::cosTheta(bRec.wi) * Frame::cosTheta(bRec.wo));

		return m_R * INV_PI + glm::vec3(spec, spec, spec);
	}

	/// Evaluate the sampling density of \ref sample() wrt. solid angles
	float Microfacet::pdf(const BSDFQueryRecord &bRec) const {
		/* Return zero when queried for illumination on the backside */
		if (Frame::cosTheta(bRec.wi) <= 0
			|| Frame::cosTheta(bRec.wo) <= 0)
			return 0.0f;

		/* Compute the half direction vector */
		glm::vec3 H = glm::normalize(bRec.wo+bRec.wi);

		/* Microfacet sampling density */
		float D_density = evalBeckmann(H) * Frame::cosTheta(H);

		/* Jacobian of the half-direction mapping */
		float dwh_dwo = 1.0f / (4.0f * glm::dot(H, bRec.wo));
			
		return (D_density * dwh_dwo) * m_ks + 
			(INV_PI * Frame::cosTheta(bRec.wo)) * (1-m_ks);
	}

	/// Sample the BRDF
	glm::vec3 Microfacet::sample(BSDFQueryRecord &bRec, const glm::vec2 &_sample) const {
		glm::vec2 sample(_sample);

		if (sample.x < m_ks) {
			/* Sample the specular component */
			sample.x /= m_ks;

			/* Sample a microsurface normal */
			glm::vec3 m = sampleBeckmann(sample);
			
			/* Compute the exitant direction */
			bRec.wo = 2 * glm::dot(m, bRec.wi) * m - bRec.wi;

		} else {
			/* Sample the diffuse component */
			sample.x = (sample.x - m_ks) / (1 - m_ks);
			bRec.wo = RTUtil::squareToCosineHemisphere(sample);
		}
		
		/* Relative index of refraction: no change */
		bRec.eta = 1.0f;

		float pdfval = pdf(bRec);
		if (pdfval == 0) {
			/* Reject samples outside of the positive 
			   hemisphere, which have zero probability */
			return glm::vec3(0, 0, 0);
		}
		
		return eval(bRec) * (Frame::cosTheta(bRec.wo) / pdfval);
	}

	glm::vec3 Microfacet::diffuseReflectance() const {
		return m_R;
	}

	// Write to a stream for debugging
    std::ostream &Microfacet::debug_write(std::ostream &ost) const {
    	ost << "Microfacet { alpha " << m_alpha
    		<< "; intIOR " << m_intIOR
    		<< "; extIOR " << m_extIOR
    		<< "; R " << m_R << "}";
    	return ost;
    }


}
