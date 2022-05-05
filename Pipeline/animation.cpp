
#define _USE_MATH_DEFINES
#include "Pipeline.hpp"
#include <glm/gtx/quaternion.hpp>
#include <RTUtil/conversions.hpp>
#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>  
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
            v = glm::mix(glm::vec3(0),kfs[0].pos,t / kfs[0].time);
        }    
    }
    else{
        // case3: has more than one keyframes
        for (int i = 0; i < kfs.size(); ++i){ 
            if (t <= kfs[i].time && i != 0){ 
                keyframe1 = kfs[i-1];
                keyframe2 = kfs[i];
                tPortion= (t - kfs[i-1].time)/(kfs[i].time- kfs[i-1].time);
                break;
            }
        }
        v = glm::mix(keyframe1.pos,keyframe2.pos,tPortion);
    }

    glm::mat4 m = glm::translate(glm::mat4(1.0f), v);
	return m;
}
   


glm::mat4 interpolateRotation(std::vector<KeyframeRot> kfs, float t) {
    KeyframeRot keyframe1;
    KeyframeRot keyframe2;
    // default not rotate
    glm::quat r = RTUtil::a2g(kfs[0].rot);
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
            glm::quat r2 = RTUtil::a2g(kfs[0].rot);
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
                glm::quat r1 = RTUtil::a2g(keyframe1.rot);
                glm::quat r2 = RTUtil::a2g(keyframe2.rot);
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
    KeyframeScale keyframe1;
    KeyframeScale keyframe2;

    glm::vec3 s = glm::vec3(1.);
    float tPortion;
    
    if (kfs.size() == 1) {
        // case1: only has one keyframe and time = 0
        if (kfs[0].time == 0){
            s = kfs[0].scale;
        } 
        // case2: has one keyframe, assume start changing scale from 0,0,0 
        else {
            s = (t / kfs[0].time) * kfs[0].scale;
        }    
    }
    else{
        // case3: has more than one keyframes
        for (int i = 0; i < kfs.size(); ++i){ 
            // start interpolation from the first non-zero sec 
            if (t <= kfs[i].time && i != 0){ 
                keyframe1 = kfs[i-1];
                keyframe2 = kfs[i];
                tPortion= (t - kfs[i-1].time)/(kfs[i].time- kfs[i-1].time);
                s = glm::mix(keyframe1.scale,keyframe2.scale,tPortion);
                break;
            }
        }
    }
    
    glm::mat4 m = glm::scale(glm::mat4(1.0f), s);
    // printm(m);
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

void Pipeline::traverseTree(const aiScene* obj, aiNode* node, glm::mat4 transMat, float t) {
    // find node has a mesh, and use nodename to find the animation of this node
    if (node != NULL){
        std::string nodeName = node->mName.C_Str();
        // find animation TRS of a node 
        if (animationOfName.find(nodeName) != animationOfName.end()) {     
            NodeAnimate na = animationOfName.at(nodeName);
            glm::mat4 TRS = getInterpolateMat(na, t);
            transMat = transMat * TRS; 

            // if this node has mesh, update mesh's transMat
            if (node->mNumMeshes > 0 ) {         
                for (int i = 0; i < node->mNumMeshes; ++i) {
                    // TOFIX: cannot use idToName[i] to reference meshname 
                    aiMesh* mesh = obj->mMeshes[node->mMeshes[i]];
                    std::string meshName = mesh->mName.C_Str();
                    transMatVec[meshName] = transMat;
                }
            }
         }
        // if this node has bone, update boneTrans
        if (boneTransMap.find(nodeName) != boneTransMap.end()){
            int boneIndex = boneTransMap[nodeName].boneId;
            glm::mat4 offsetMat = boneTransMap[nodeName].mat;
            glm::mat4 globalBoneMat = transMat * offsetMat;
            boneTrans.push_back(globalBoneMat);
        }
        
        for (int i = 0; i < node->mNumChildren; ++i) {
            traverseTree(obj, node->mChildren[i],transMat, t);
        }
    }
}

void Pipeline::playMeshAnimation(){
        float currentFrame = glfwGetTime();
        float t = std::fmod(currentFrame - startTime, totalTime);

        Assimp::Importer importer;
        const aiScene* obj = importer.ReadFile(GlobalPath, aiProcess_LimitBoneWeights | aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
        // if the scene has animation, traverse the tree to update TRS
        if (animationOfName.size() > 0){
            boneTrans.clear();
            traverseTree(obj, obj->mRootNode, glm::mat4(1.f), t);
        }
}
