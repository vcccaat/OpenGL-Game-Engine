#ifndef CAMERA_HEADER
#define CAMERA_HEADER

// By Eston

// TODO: rename zoom, rename scale->height in ortho
// TODO: consisder renaming vertical and up to more standard names

#define _USE_MATH_DEFINES
#undef near
#undef far
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common.hpp"

namespace RTUtil {


/// A representation of a Camera in a 3D space.
/// This class is designed to be compatible with Nanogui and OpenGL, but does
/// not depend on them.
class Camera {
public:
  /// Initalizes a Camera.
  /// @param eye The starting location of the Camera.
  /// @param target The point in the center of the Camera's focus.
  /// @param vertical The direction opposite of gravity.
  /// @param aspect The aspect ratio of the Camera (width/height).
  /// @param near The distance to the Camera's near plane.
  /// @param far The distance to the Camera's far plane.
  Camera(const glm::vec3& eye, const glm::vec3& target,
         const glm::vec3& vertical, float aspect, float near, float far) :
  m_eye(eye), m_target(target), m_vertical(vertical), m_aspect(aspect),
  m_near(near), m_far(far) {
    // update right for the first time, since it is used by updateFrame()
    m_negGaze = glm::normalize(eye - target);
    m_right = glm::cross(vertical, m_negGaze);
    updateFrame();
  }

  virtual ~Camera() { }

  /// @return The Camera's current view matrix.
  virtual const glm::mat4& getViewMatrix() const {
    return viewMat();
  }

  /// @return The Camera's current projection matrix.
  virtual const glm::mat4& getProjectionMatrix() const {
    return projMat();
  }

  /// @return The product of the Camera's current projection matrix and its
  ///         view matrix.
  virtual glm::mat4 getViewProjectionMatrix() const {
    return viewProjMat();
  }

  /// @return The current location of the Camera.
  const glm::vec3& getEye() const {
    return eye();
  }

  /// @return The current location of the Camera's focus.
  const glm::vec3& getTarget() const {
    return target();
  }

  /// @return The current up direction according to the Camera's orientation.
  const glm::vec3& getUp() const {
    return up();
  }

  /// @return The current right direction according to the Camera's orientation.
  const glm::vec3& getRight() const {
    return right();
  }

  /// @return The current vertical direction according to the Camera's orientation.
  const glm::vec3& getVertical() const {
    return vertical();
  }

  /// @return The current aspect ratio of the camera.
  float getAspectRatio() const {
    return aspect();
  }

  /// Sets the aspect ratio of the Camera.
  /// @param ratio The new aspect ratio (width/height).
  virtual void setAspectRatio(float ratio) {
    aspect() = ratio;
  }

  /// Sets the location of the Camera.
  /// @param eye The new location of the Camera.
  virtual void setEye(const glm::vec3& eye) {
    this->eye() = eye;
  }

  /// Sets the location of the Camera's focus.
  /// @param target The new location of the Camera's focus.
  virtual void setTarget(const glm::vec3& target) {
    this->target() = target;
  }


protected:
  // memoized values
  mutable bool frameDirty = true;
  mutable glm::vec3 m_up;
  mutable glm::vec3 m_right;
  mutable glm::vec3 m_negGaze;

  mutable bool viewMatDirty = true;
  mutable glm::mat4 m_viewMat;

  mutable bool projMatDirty = true;
  mutable glm::mat4 m_projMat;

  const glm::vec3& eye() const { return m_eye; }
  glm::vec3& eye() {
    frameDirty = true;
    return m_eye;
  }  
  const glm::vec3& target() const { return m_target; }
  glm::vec3& target() {
    frameDirty = true;
    return m_target;
  }
  const glm::vec3& vertical() const { return m_vertical; }
  glm::vec3& vertical() {
    frameDirty = true;
    return m_vertical;
  }

  const float& aspect() const { return m_aspect; }
  float& aspect() {
    projMatDirty = true;
    return m_aspect;
  }
  const float& near() const { return m_near; }
  float& near() {
    projMatDirty = true;
    return m_near;
  }
  const float& far() const { return m_far; }
  float& far() {
    projMatDirty = true;
    return m_aspect;
  }

  const glm::vec3& up() const {
    updateFrameIfNecessary();
    return m_up;
  }
  const glm::vec3& right() const {
    updateFrameIfNecessary();
    return m_right;
  }
  const glm::vec3& negGaze() const {
    updateFrameIfNecessary();
    return m_negGaze;
  }

  const glm::mat4& viewMat() const {
    updateViewMatIfNecessary();
    return m_viewMat;
  }
  const glm::mat4& projMat() const {
    updateProjMatIfNecessary();
    return m_projMat;
  }
  glm::mat4 viewProjMat() const {
    updateViewMatIfNecessary();
    updateProjMatIfNecessary();
    return m_projMat * m_viewMat;
  }

  virtual void updateFrame() const {
    m_negGaze = glm::normalize(eye() - target());
    m_right = glm::normalize(glm::cross(vertical(), m_negGaze));
    m_up = glm::cross(m_negGaze, m_right);
    frameDirty = false;
    viewMatDirty = true;
  }
  virtual void updateViewMat() const {
    m_viewMat = glm::lookAt(eye(), target(), up());
  }
  virtual void updateProjMat() const =0;

private:
  glm::vec3 m_eye;
  glm::vec3 m_target;
  glm::vec3 m_vertical;

  float m_aspect;
  float m_near;
  float m_far;

  void updateFrameIfNecessary() const {
    if (frameDirty) {
      updateFrame();
      frameDirty = false;
      viewMatDirty = true;
    }
  }

  void updateViewMatIfNecessary() const {
    if (frameDirty || viewMatDirty) {
      updateViewMat();
      viewMatDirty = false;
    }
  }

  void updateProjMatIfNecessary() const {
    if (projMatDirty) {
      updateProjMat();
      projMatDirty = false;
    }
  }

};

/// A pinhole perspective Camera.
class PerspectiveCamera : public Camera {
public:
  /// Initalizes a PerspectiveCamera.
  /// @param eye The starting location of the Camera.
  /// @param target The point in the center of the Camera's focus.
  /// @param vertical The direction opposite of gravity.
  /// @param aspect The aspect ratio of the Camera (width/height).
  /// @param near The distance to the Camera's near plane.
  /// @param far The distance to the Camera's far plane.
  /// @param fovy The full field of view angle in radians.
  PerspectiveCamera(const glm::vec3& eye, const glm::vec3& target,
    const glm::vec3& vertical, float aspect, float near, float far,
    float fovy) :
  Camera(eye, target, vertical, aspect, near, far), fovy(fovy) { }


  /// Gets the PerspectiveCamera's current field of view.
  /// @return The full field of view angle in radians.
  float getFOVY() const { return fovy; }

  /// Sets the PerspectiveCamera's current field of view.
  /// @param fovy The full field of view angle in radians.
  void setFOVY(float fovy) { 
    this->fovy = fovy;
    projMatDirty = true;
  }

protected:
  float fovy = 0.785398f; // 45 deg

  /// Updates the PerspectiveCamera's projection matrix.
  /// The calculations are based on the current configurations of the camera.
  virtual void updateProjMat() const {
    m_projMat = glm::perspective(fovy, aspect(), near(), far());
  }
};


/// An orthographic Camera.
class OrthoCamera : public Camera {
public:
  /// Initalizes a OrthoCamera.
  /// @param eye The starting location of the Camera.
  /// @param target The point in the center of the Camera's focus.
  /// @param vertical The direction opposite of gravity.
  /// @param aspect The aspect ratio of the Camera (width/height).
  /// @param near The distance to the Camera's near plane.
  /// @param far The distance to the Camera's far plane.
  /// @param scale The scale of the OrthoCamera's viewport.
  OrthoCamera(const glm::vec3& eye, const glm::vec3& target,
              const glm::vec3& vertical, float aspect, float near,
              float far, float scale) :
  Camera(eye, target, vertical, aspect, near, far), scale(scale) { }

  /// @return The current scale OrthoCamera's viewport.
  float getScale() const { return scale; }

  /// @param scale The new scale of the OrthoCamera's viewport.
  void setScale(float scale) {
    this->scale = scale;
    projMatDirty = true;
  }

  /// "Zooms" the OrthoCamera by changing the scale of the viewport, rather than
  /// moving the Camera itself (which would have no visual effect).
  /// @param delta The viewport scale will be scaled by <tt>1.0 - delta</tt>.
  virtual void zoom(float delta) {
    scale *= 1.0 - delta;
    projMatDirty = true;
  }

protected:

  float scale = 10.0f;

  /// Updates the OrthoCamera's projection matrix.
  /// The calculations are based on the current configurations of the camera.
  virtual void updateProjMat() const {
    m_projMat = glm::mat4(1.0);
    m_projMat[0][0] = 2.0f / scale;
    m_projMat[1][1] = 2.0f / (scale / aspect());
    m_projMat[2][2] = -2.0f / (far() - near());
    m_projMat[2][3] = -(far() + near()) / (far() - near());
  }
};

} // namespace


// Alternative implementation for reference

/*

// Camera.hpp



class Camera {
public:

    Camera(Vector3 position, Vector3 lookDir, Vector3 upVector, 
           float hFov, float aspect, float near, float far) 
    : position(position), lookDir(lookDir), upVector(upVector), 
      hFov(hFov), aspect(aspect), near(near), far(far) {
        init();
    }

    Camera() 
    : position(3, 4, 5), lookDir(-3, -4, -5), upVector(0, 1, 0), hFov(30 * M_PI / 180.0), aspect(1) {
        init();
    }

    // uv in [0,1]^2
    Ray getRay(const Vector2 &uv) const {
        float a = (2 * uv[0] - 1) * tan(hFov / 2);
        float b = (2 * uv[1] - 1) * tan(hFov / 2) / aspect;
        Vector3 rayDir = a * u + v * b - w;
        return Ray(position, rayDir);
    }

    inline void setPose(Vector3 position, Vector3 lookDir, Vector3 upVector) {
        this->position = position;
        this->lookDir = lookDir;
        this->upVector = upVector;
        init();
    }

    inline void setIntrinsics(float hFov, float aspect, float near, float far) {
        this->hFov = hFov;
        this->aspect = aspect;
        this->near = near;
        this->far = far;
        init();
    }

private:

    Vector3 position, lookDir, upVector;
    float hFov, aspect;
    float near, far;

    void init() {
        // Projection matrix depends only on intrinsics
        projMatrix <<

        Vector3 u, v, w;
        w = (-lookDir).normalized();
        u = upVector.cross(w).normalized();
        v = w.cross(u);
    }
};

*/

#endif // CAMERA_HEADER
