
#pragma once

void print(glm::vec3 v){
    std::cout<<"["<<v[0]<<","<<v[1]<<","<<v[2]<<"]\n";
}
void printv4(glm::vec4 v){
    if (v[0] < .0000001) v[0] = 0;
    if (v[1] < .0000001) v[1] = 0;
    if (v[2] < .0000001) v[2] = 0;
    if (v[3] < .0000001) v[3] = 0;
    std::cout<<"["<<v[0]<<","<<v[1]<<","<<v[2]<<","<<v[3]<<"]\n";
}

void printm(glm::mat4 m){
    std::cout << "[\n";
    printv4(glm::vec4(m[0][0], m[1][0], m[2][0],m[3][0]));
    printv4(glm::vec4(m[0][1], m[1][1], m[2][1],m[3][1]));
    printv4(glm::vec4(m[0][2], m[1][2], m[2][2],m[3][2]));
    printv4(glm::vec4(m[0][3], m[1][3], m[2][3],m[3][3]));
    std::cout << "]\n";
}

