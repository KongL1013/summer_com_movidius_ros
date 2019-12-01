//
// Created by stumbo on 18-8-21.
//

#ifndef COMP_STRATEGY_TOOLS_H
#define COMP_STRATEGY_TOOLS_H

#include <sstream>
#include <string>
#include "ros/ros.h"

#include <std_msgs/String.h>
#include <std_msgs/UInt8.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <px4_autonomy/Takeoff.h>
#include <px4_autonomy/Velocity.h>
#include <px4_autonomy/Position.h>
#include <cmath>

typedef struct vec3f_s {

    float x;
    float y;
    float z;
    ros::Time time;

    vec3f_s(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3f_s() : x(0.0f), y(0.0f), z(0.0f) {}

    vec3f_s operator-(const vec3f_s &b) const {
        vec3f_s res{};
        res.x = this->x - b.x;
        res.y = this->y - b.y;
        res.z = this->z - b.z;
        res.time = ros::Time::now();
        return res;
    }

    vec3f_s operator+(const vec3f_s &b) const {
        vec3f_s res{};
        res.x = this->x + b.x;
        res.y = this->y + b.y;
        res.z = this->z + b.z;
        res.time = ros::Time::now();
        return res;
    }

    vec3f_s operator*(const float &b) const {
        vec3f_s res;
        res.x = this->x * b;
        res.y = this->y * b;
        res.z = this->z * b;
        res.time = ros::Time::now();
        return res;
    }

    vec3f_s &operator=(const px4_autonomy::Position &b) {
        this->x = b.x;
        this->y = b.y;
        this->z = b.z;
        this->time = ros::Time::now();
        return *this;
    }

    px4_autonomy::Position toPosCmd() const {
        px4_autonomy::Position posCmd;
        posCmd.x = this->x;
        posCmd.y = this->y;
        posCmd.z = this->z;
        posCmd.yaw = 3.1415926 / 2.0;
        posCmd.header.stamp = ros::Time::now();
        return posCmd;
    }

    px4_autonomy::Velocity toVelCmd() const {
        px4_autonomy::Velocity velCmd;
        velCmd.x = this->x;
        velCmd.y = this->y;
        velCmd.z = this->z;
        velCmd.yaw_rate = 0;
        velCmd.header.stamp = ros::Time::now();
        return velCmd;
    }

    float distXY() const {
        return sqrtf(x * x + y * y);
    }

    float distXZ() const {
        return sqrtf(x * x + z * z);
    }

    float len() const {
        return sqrtf(x * x + z * z + y * y);
    }

    std::string toString() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) <<"(" << x << " : " << y << " : " << z << ")";
        std::string msg;
        std::getline(ss, msg);
        return msg;
    }

    void set(float X, float Y, float Z) {
        x = X;
        y = Y;
        z = Z;
    }

    void set(vec3f_s a) {
        x = a.x;
        y = a.y;
        z = a.z;
    }

    static bool isXBigger(const vec3f_s &a, const vec3f_s &b) {
        return a.x < b.x;
    }

    float dot(const vec3f_s &a) const {
        return this->x * a.x + this->y * a.y + this->z * a.z;
    }

    float angleToVec(const vec3f_s &a) const {
        return acosf(this->dot(a) / this->len() / a.len());
    }
} vec3f_t;

float constrainF(float value, float max, float min) {
    if (value > max) {
        return max;
    } else if (value < min) {
        return min;
    }
    return value;
}

/**
 * the point on a line
 * @param x target x
 * @param x1 line point 1
 * @param x2 line point 2
 * @param y1 line point 1
 * @param y2 line point 2
 * @return coor y on the line with the input x
 */
float slopeCal(float x, float x1, float x2, float y1, float y2) {
    if (x1 == x2) {
        return (y1 + y2) / 2.0f;
    }

    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    if (x < x1) {
        return y1;
    }
    if (x > x2) {
        return y2;
    }

    float slope = (y2 - y1) / (x2 - x1);
    return y1 + (x - x1) * slope;
}

#endif //COMP_STRATEGY_TOOLS_H
