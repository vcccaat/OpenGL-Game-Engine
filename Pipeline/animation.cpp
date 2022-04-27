
#define _USE_MATH_DEFINES
#include "Pipeline.hpp"
#include <glm/gtx/quaternion.hpp>
#include "Helper.hpp"


glm::mat4 interpolatePosition(std::vector<KeyframePos> kfs, float t) {
    KeyframePos keyframe1;
    KeyframePos keyframe2;
    glm::vec3 v;
    float tPortion;
    
    if (kfs.size() == 1) {
        // case1: only has one keyframe and time = 0
        if (kfs[0].time == 0){
            v = kfs[0].pos;
        } 
        // case2: has one keyframe, assume start pos at 0,0,0
        else {
            v = glm::vec3(0) + (t / kfs[0].time) * kfs[0].pos;
        }    
    }
    else{
        // case3: has more than one keyframes
        for (int i = 0; i < kfs.size(); ++i){
            if (t < kfs[i].time){
                keyframe1 = kfs[i-1];
                keyframe2 = kfs[i];
                tPortion= (t - kfs[i-1].time)/(kfs[i].time- kfs[i-1].time);
                break;
            }
        }
        v = keyframe1.pos + tPortion * (keyframe2.pos - keyframe1.pos);
    }

	glm::mat4 m( 1.0f );
    m[3][0] = v[0];
    m[3][1] = v[1];
    m[3][2] = v[2];
    // printm(m);
	return m;
}

glm::mat4 interpolateRotation(std::vector<KeyframeRot> kfs, float t) {
    KeyframeRot keyframe1;
    KeyframeRot keyframe2;
    // default not rotate
    glm::quat r = glm::quat(kfs[0].rot.w,kfs[0].rot.x,kfs[0].rot.y,kfs[0].rot.z );
    float tPortion;
    
    if (kfs.size() == 1) {
        // case1: only has one keyframe and time = 0
        if (kfs[0].time == 0){
            r = r;
        } 
        // case2: has one keyframe, assume start rot at 0,0,0,0
        else {
            tPortion = t/kfs[0].time;
            glm::quat r1 = glm::quat(0.,0.,0.,0.);
            glm::quat r2 = glm::quat(kfs[0].rot.w,kfs[0].rot.x,kfs[0].rot.y,kfs[0].rot.z );
            r = glm::mix(r1,r2,tPortion);
        }    
    }
    else{
        // case3: has more than one keyframes
        for (int i = 0; i < kfs.size(); ++i){ 
            // rotation keyframe time might start at non-zero
            if (t < kfs[i].time && i == 0){ 
                r = r;
                break;
            }
            // start interpolation from the first non-zero sec 
            if (t < kfs[i].time && i != 0){ 
                keyframe1 = kfs[i-1];
                keyframe2 = kfs[i];
                tPortion= (t - kfs[i-1].time)/(kfs[i].time- kfs[i-1].time);
                glm::quat r1 = glm::quat(keyframe1.rot.w,keyframe1.rot.x,keyframe1.rot.y,keyframe1.rot.z );
                glm::quat r2 = glm::quat(keyframe2.rot.w,keyframe2.rot.x,keyframe2.rot.y,keyframe2.rot.z );
                // printf(": %f, %f, %f, %f\n" , r1.x , r1.y , r1.z,r1.w ) ;
                // printf(": %f, %f, %f, %f\n" , r2.x , r2.y , r2.z,r2.w ) ;
                r = glm::mix(r1,r2,tPortion);
                break;
            }
        }
    }
    
    glm::mat4 m = glm::toMat4(r);
    // printm(m);
    return m;
}

glm::mat4 interpolateScaling(std::vector<KeyframeScale> kfs, float t) {
  // TODO
    glm::mat4 m = glm::mat4(1.0f);
    return m;
}

glm::mat4 getInterpolateMat(NodeAnimate node, float t) {  
    glm::mat4 m1 = glm::mat4(1.f);
    glm::mat4 rotation = m1;
    glm::mat4 scale = m1;
    glm::mat4 translation = m1;

    translation = interpolatePosition(node.keyframePos, t);
    rotation = interpolateRotation(node.keyframeRot, t);
    scale = interpolateScaling(node.keyframeScale, t);
    
    return translation * rotation * scale;
}

void Pipeline::traverseTree(aiNode* node, glm::mat4 transMat, int counter, float t) {
    // find node has a mesh, and use nodename to find the animation of this node
    if (node != NULL){
        std::string name = node->mName.C_Str();
        if (animationOfName.find(name) != animationOfName.end()) { 
            // std::cout << name << "\n";          
            NodeAnimate na = animationOfName.at(name);
            glm::mat4 TRS = getInterpolateMat(na, t);
            // printm(TRS);
            // Update model matrix
            transMat = transMat * TRS;  
        }
        if (node->mNumMeshes > 0 ) {
             for (int i = 0; i < node->mNumMeshes; ++i) {
                transMatVec[name] = transMat;
                counter += 1;
            }
        }
        
        for (int i = 0; i < node->mNumChildren; ++i) {
            traverseTree(node->mChildren[i],transMat, counter, t);
        }
        }

    }

