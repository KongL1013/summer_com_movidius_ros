//
// Created by stumbo on 18-8-20.
//

#ifndef TARGET_H
#define TARGET_H

#include <vector>
#include <utility>
#include "tools.h"
#include <algorithm>


enum SearchDirection {TOWARD_LEFT = -1, UNSET = 0, TOWARD_RIGHT = 1};

enum TargetType{ CIRCLE, PILLAR };

struct Target {
    SearchDirection toDir = UNSET; //找的方向
    int ID;//板子和柱子的全局编号
    vec3f_t postion;//目标的绝对位置
    TargetType type;//板子或者柱子
    std::vector<vec3f_t> poses;    //CHANGE TO POSES
    float circleHeight;
    float possibleX = -6.5f;
    size_t possiblePose = 0;

    std::pair<size_t, float> checkForClosestPose(vec3f_t detected) {
        float minDist = 10.0f;
        size_t minNo;
        for (size_t i = 0; i < poses.size(); i ++) {
            vec3f_t toDetect(poses[i].x, poses[i].y, 0);
            float dist = (toDetect - detected).distXY();
            if (dist < minDist) {
                minDist = dist;
                minNo = i;
            }
        }
        return std::make_pair(minNo, minDist);
    }

    Target(int ID, vec3f_t postion,TargetType type)
            : ID(ID), postion(postion), type(type)
    {}

    Target(int ID, vec3f_t postion,TargetType type, SearchDirection toDir)
            : ID(ID), postion(postion), type(type), toDir(toDir)
    {}

    Target(int ID, TargetType type, SearchDirection toDir)
            : ID(ID), type(type), toDir(toDir)
    {}

};
#endif //TARGET_H
