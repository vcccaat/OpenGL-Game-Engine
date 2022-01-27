/*
 * Cornell CS5625
 * RTUtil library
 * 
 * Interface for parsing additional scene data for the Ray Tracing assingment from
 * a JSON file.  
 * 
 * Author: srm, Spring 2020
 */
#pragma once

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <glm/glm.hpp>

#include "common.hpp"

// forward decl
namespace RTUtil {

    // An enumeration to distinguish different types of lights
    enum LightType {
        Point, Area, Ambient
    };

    /**
     * Holds the information about one light source in the scene.  All have a node name
     * and a type, and depending on the type, a subset of the remaining fields hold the
     * relevant information about the light.
     */
    struct RTUTIL_EXPORT LightInfo {

        // The name of the node corresponding to this light.  If there is a node in the scene
        // by this name, then the transformation of that node applies to this light.  If there
        // is no node by that name, then the geometry of this light is to be interpreted in
        // world coordinates.
        std::string nodeName;

        // The type of this light
        LightType type;

        // For Point and Area lights: the power of the light source.  Point sources emit this
        // power isotropically, and area sources are one-sided Lambertian emitters.
        glm::vec3 power;

        // For Point and Area lights: the position of the (center of the) light source.
        glm::vec3 position;

        // For Area lights: the direction normal to the light's rectangular surface, and the
        // vector that defines the orientation of the height dimension of the rectangle (in 
        // the same way as for a standard camera specification)
        glm::vec3 normal, up;

        // For Area lights: the width and height of the rectangular source
        glm::vec2 size;

        // For Ambient lights: the (constant) ambient radiance
        glm::vec3 radiance;

        // For Ambient lights: the maximum distance of an occluder for ambient light.  Setting
        // this to positive infinity gives standard ambient occlusion.
        float range;    
    };


    /**
     * Holds all the auxiliary information parsed by this module.
     */
    struct RTUTIL_EXPORT SceneInfo {
        // A list of the lightInfo data for each light in the scene
        std::vector<std::shared_ptr<LightInfo>> lights;

        // The background radiance of the scene (for rays that miss all geometry)
        glm::vec3 backgroundRadiance;
    };


    /**
     * Parse scene info from a file.  This is the main entry point for this module, and the
     * normal usage is to create a default SceneInfo and pass a reference to it into this
     * function, which reads the file and fills in the structure.
     * @arg infoPath The pathname to the JSON scene information file to be parsed.
     * @arg info The destination for the resulting data.
     * @return true if the file was parsed successfully, false on error. 
     */
    RTUTIL_EXPORT bool readSceneInfo(const std::string infoPath, SceneInfo &info);

}



