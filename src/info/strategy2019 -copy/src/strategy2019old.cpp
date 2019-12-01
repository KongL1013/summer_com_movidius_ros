#include "ros/ros.h"
#include <std_msgs/String.h>
#include <std_msgs/UInt8.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/Point.h>
#include <px4_autonomy/Takeoff.h>
#include <px4_autonomy/Velocity.h>
#include <px4_autonomy/Position.h>
#include <dd/pubmsg.h>
#include <dd/total.h>
#include <std_msgs/Float32MultiArray.h>

#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <iostream>
#include "tools.h"
#include "Target.h"


#define FAKE_UNMOVE_TEST_ENABLE false
#define NO_ACCURATE_X_MODE false

using std::vector;
using ros::Publisher;


float REFRESH_RATE = 10;
float REFRESH_PERIOD;

float BOARD_AIM_HEIGHT = 0.5;
float GO_THROUGH_CIRCLE_HEIGHT = 1.45;
float PILLAR_AIM_HEIGHT = 1.0;
float GO_THROUGH_CIRCLE_DISTANCE = 1.5;
float FLYING_OVER_PILLAR_HEIGHT = 1.5;


float MAX_XY_ACC = 2.0;
float MAX_XY_VEL = 1.0;
float MAX_Z_ACC = 2.0;
float MAX_Z_VEL = 0.81;
float HEIGHT_MAX = 3;
float HEIGHT_MIN = 0.35;

float TOLLERANCE_Z;
float TOLLERANCE_XY;

float LARGE_TOLLERANCE_Z = 0.3;
float LARGE_TOLLERANCE_XY = 0.3;
float SMALL_TOLLERANCE_Z = 0.1;
float SMALL_TOLLERANCE_XY = 0.15;

float LEFT_MOVING_MAX = 3.6;
float RIGHT_MOVING_MAX = -3.6;

float LEFT_NET_BOUND = 3.6;
float RIGHT_NET_BOUND = -3.6;


enum STATE_TYPE {
    TARGET_CIRCLE_NUM,
    STATE_DESTPOS_ARRIVED,
    STAGE_NUM,
    STARTPOS_IS_USEFUL,
    TARGET_DETECTED,
    TARGETPOS_UPDATED,
    SONAR_USEFUL
};


std::map<STATE_TYPE, int> state
        {
                {TARGET_CIRCLE_NUM,     0},
                {STATE_DESTPOS_ARRIVED, 0},
                {STAGE_NUM,             0},
                {STARTPOS_IS_USEFUL,    0},
                {TARGET_DETECTED,       0},
                {TARGETPOS_UPDATED,     0},
                {SONAR_USEFUL,     0},
        };
std::map<STATE_TYPE, int> stateold;

Publisher pub_Pose;
Publisher pub_Vel;
Publisher pub_TakeOff;
Publisher pub_camCmd;
Publisher pub_FakePose;
Publisher pub_FakeStatus;

vec3f_t currentPosIdeal = {0.0, 0.0, 0.0};
vec3f_t setpointPosIdeal;

vec3f_t currentPX4Pos = {0.0, 0.0, 0.0};
vec3f_t currentVel = {0.0, 0.0, 0.0};
//vec3f_t destPX4Pos = {0.0, 0.0, 0.0};
vec3f_t startPX4Pos;
vec3f_t originPX4Pos = {0, 0, 0};

vec3f_t targetPos;
vec3f_t BIAS_TO_YELLOW_BOARD, BIAS_TO_PILLAR;

vector<Target> Targets;

std::vector<SearchDirection> circleDirection;

uint8_t autonomy_status;

u_int8_t MoveMethodBetween[17][17];

bool QRStarted = false;

int currentTargetID = 0;


//region function declaration
void getParas(ros::NodeHandle &n);

void InitPlaces();


void updatePosToIdeal();

void sendAutonomyPos();


void calNext(vec3f_t velToThisDirection, vec3f_t disToThisDirection, vec3f_t &outputvel, vec3f_t &outputdis,
             float MAX_ACC, float MAX_VEL);

void sendGentlyStopPos();

void MoveBy(bool is_precise, float disX, float disY, float disZ = 0.0);

void MoveTo(bool is_precise, vec3f_t target);

void MoveToZ(bool is_precise, float Z);

void StartMoveBy(bool is_precise, float disX, float disY, float disZ = 0.0);

void StartMoveTo(bool is_precise, vec3f_t target);

void EndMove();

void PillarFromTo(int startPillar, int endPillar);

void aimToApron();

void aimToCircle();

void aimToPillar();

void CB_PX4Pose(const px4_autonomy::Position &msg);

void CB_status(const std_msgs::UInt8 &msg);

void CB_Camera(const pub_rospy::total::ConstPtr &msg);

void CB_pillar(const std_msgs::Float32MultiArray &msg);

void CB_Sonar(const geometry_msgs::PointConstPtr &msg);

void CB_outputTimer(const ros::TimerEvent &e);

void CB_camSigTimer(const ros::TimerEvent &e);

void TakeOff();

void Hover();


int CheckSignal(std::string);

//void SendCamCMD(CamMode);

void Land();


//endregion

int main(int argc, char **argv) {
    ros::init(argc, argv, "strategy2019");
    ros::NodeHandle n;

    getParas(n);

    InitPlaces();

    ros::Timer outputTimer = n.createTimer(ros::Duration(1), CB_outputTimer);
    ros::Timer camSigTimer = n.createTimer(ros::Duration(1 / 30.0), CB_camSigTimer);

    ros::Subscriber sub_localPos = n.subscribe("/px4/pose", 1, CB_PX4Pose);
    ros::Subscriber sub_status = n.subscribe("/px4/status", 1, CB_status);

    ros::Subscriber sub_camPose_qr = n.subscribe("/total_info", 1, CB_Camera);

    ros::Subscriber sub_camPose_pl = n.subscribe("/pillar", 1, CB_pillar);


    if (NO_ACCURATE_X_MODE)
    {
        ros::Subscriber sub_sonar = n.subscribe("/sonar_distance", 1, CB_Sonar);
    }

    pub_Pose = n.advertise<px4_autonomy::Position>("/px4/cmd_pose", 1);
    pub_Vel = n.advertise<px4_autonomy::Velocity>("/px4/cmd_vel", 1);
    pub_TakeOff = n.advertise<px4_autonomy::Takeoff>("/px4/cmd_takeoff", 1);
    pub_camCmd = n.advertise<std_msgs::UInt8>("/sig_info", 1);

    if (FAKE_UNMOVE_TEST_ENABLE)
    {
        ROS_WARN_STREAM("[FAKE] You are now at #FAKE TEST# mode!");
        pub_FakePose = n.advertise<px4_autonomy::Position>("/px4/pose", 1);
        pub_FakeStatus = n.advertise<std_msgs::UInt8>("/px4/status", 1);
    }

    if (NO_ACCURATE_X_MODE)
    {
        ROS_WARN_STREAM("[NO X] You are now at #NO ACCURATE X# mode!");
    }

    ros::Rate loop_rate(REFRESH_RATE);


    ROS_WARN_STREAM("[CORE] Init complete");


    ros::spinOnce();
    originPX4Pos = currentPX4Pos;

    ////Take off
    TakeOff();
    ros::Duration(1).sleep();
    ros::spinOnce();


    ////Stage 0 : Aim to the apron
    state.at(STAGE_NUM) = 0;
    ros::Duration(1).sleep();   //
    ros::spinOnce();
    aimToApron();
    state.at(STARTPOS_IS_USEFUL) = 0;
    updatePosToIdeal();
    currentPosIdeal = {0, 0, currentPX4Pos.z};


    ////Stage 1 : Go through circles
//    state.at(STAGE_NUM) = 1;
//    ros::Duration(3).sleep();
//
//
//    int targetnum = 1;
//    while (targetnum < 3) //todo
//    {
//        ROS_WARN_STREAM("current target: " << targetnum << " !!!!!!!!!!!!!!!!!!!!");
//        state.at(TARGET_CIRCLE_NUM) = targetnum;
//
//        MoveToZ(true, BOARD_AIM_HEIGHT);
//
//        state.at(TARGET_DETECTED) = 0;
//        ros::spinOnce();
//        ros::Duration(0.3).sleep();
//
//
//        while (ros::ok())
//        {
//            if (Targets[targetnum].toDir == TOWARD_LEFT ||
//                (Targets[targetnum].toDir == UNSET && currentPosIdeal.y >= 0))
//            {
//                ROS_WARN_STREAM("Going LEFT! ");
//                StartMoveBy(false, 0, LEFT_MOVING_MAX - currentPosIdeal.y);
//            } else if (Targets[targetnum].toDir == TOWARD_RIGHT ||
//                       (Targets[targetnum].toDir == UNSET && currentPosIdeal.y < 0))
//            {
//                ROS_WARN_STREAM("Going RIGHT!");
//                StartMoveBy(false, 0, RIGHT_MOVING_MAX - currentPosIdeal.y);
//            } else
//            {
//                ROS_ERROR("StartMoveBy WRONG! ");
//            }
//            while (state.at(STATE_DESTPOS_ARRIVED) == 0 && ros::ok())
//            {
//                state.at(SONAR_USEFUL) = 1;
//                ROS_WARN_STREAM_DELAYED_THROTTLE(2, "LOOKING FOR " << targetnum);
//                ros::spinOnce();
//                updatePosToIdeal();
//                vec3f_t error = currentPosIdeal - setpointPosIdeal;
//                if (fabsf(error.x) < TOLLERANCE_XY && fabsf(error.y) < TOLLERANCE_XY && fabsf(error.z) < TOLLERANCE_Z)
//                {
//                    state.at(STATE_DESTPOS_ARRIVED) = 1;
//                    break;
//                }
//                if (state.at(TARGET_DETECTED) == 1)
//                {
//                    break;
//                }
//                sendAutonomyPos();
//
//                loop_rate.sleep();
//            }
//            if (state.at(TARGET_DETECTED) == 1)
//            {
//                EndMove();
//                break;
//            }
//            if (state.at(STATE_DESTPOS_ARRIVED) == 1)
//            {
//                if (currentPosIdeal.y >= 0)
//                {
//                    Targets[targetnum].toDir = TOWARD_RIGHT;
//                } else
//                {
//                    Targets[targetnum].toDir = TOWARD_LEFT;
//                }
//                EndMove();
//                state.at(STATE_DESTPOS_ARRIVED) = 0;
//            }
//        }
//
//        state.at(SONAR_USEFUL) = 0;
//
//        aimToCircle();
//
//
//        if (state.at(TARGET_DETECTED) == 1)
//        {
//            currentPosIdeal.x = Targets[targetnum].postion.x + BIAS_TO_YELLOW_BOARD.x;
//
//            MoveToZ(true, GO_THROUGH_CIRCLE_HEIGHT);
//            MoveBy(true, GO_THROUGH_CIRCLE_DISTANCE, 0);        //go through the circle
//            ROS_WARN_STREAM("Go through circle" << targetnum << " COMPLETED!!!!!!!!!!!!!!");
//            targetnum++;
//        }
//    }

    ////Stage 2
    state.at(STAGE_NUM) = 2;
    ros::Duration(3).sleep();

    int now_wood_index, next_wood_index;

    next_wood_index = 7;

//    MoveTo first pillar   //todo

    MoveToZ(true, PILLAR_AIM_HEIGHT);
    state.at(TARGET_DETECTED) = 0;


    while (next_wood_index != 15 && ros::ok())
    {
        now_wood_index = next_wood_index;
        ROS_WARN_STREAM("arrive at" << now_wood_index << " pillar!");

        aimToPillar();

        ROS_WARN_STREAM("aimToPillar end.");

        currentPosIdeal.set(Targets[now_wood_index].postion + BIAS_TO_PILLAR);

        next_wood_index = now_wood_index + 1;

        ROS_WARN_STREAM("flying to" << next_wood_index << " pillar!");
        PillarFromTo(now_wood_index, next_wood_index);
        state.at(TARGET_DETECTED) = 0;
    }


    return 0;
}

void MoveBy(bool is_precise, float disX, float disY, float disZ) {
    StartMoveBy(is_precise, disX, disY, disZ);

    ros::Rate loop_rate(REFRESH_RATE);
    while (state.at(STATE_DESTPOS_ARRIVED) == 0 && ros::ok())
    {
        ros::spinOnce();
        updatePosToIdeal();
        vec3f_t error = currentPosIdeal - setpointPosIdeal;
        if (fabsf(error.x) < TOLLERANCE_XY && fabsf(error.y) < TOLLERANCE_XY && fabsf(error.z) < TOLLERANCE_Z)
        {
            state.at(STATE_DESTPOS_ARRIVED) = 1;
            break;
        }
        sendAutonomyPos();

        loop_rate.sleep();
    }

    EndMove();

}

inline void MoveTo(bool is_precise, vec3f_t target) {
    MoveBy(is_precise, target.x - currentPosIdeal.x, target.y - currentPosIdeal.y, target.z - currentPosIdeal.z);
}


inline void MoveToZ(bool is_precise, float Z) {

    MoveBy(is_precise, 0, 0, Z - currentPX4Pos.z);
    currentPosIdeal.z = currentPX4Pos.z;
}
//
//inline void MoveTo(float targetX, float targetY, float targetZ, bool usingVelSP) {
//
//}
//
//inline void MoveBy(vec3f_t dist, bool usingVelSp) {
//    MoveBy(dist.x, dist.y, dist.z, usingVelSp);
//}
//
//inline void MoveTo(vec3f_t target, bool usingVelSp) {
//    vec3f_t dist = target - currentPosIdeal;
//    MoveBy(dist, usingVelSp);
//}

void StartMoveBy(bool is_precise, float disX, float disY, float disZ) {
    if (is_precise)
    {
        TOLLERANCE_Z = SMALL_TOLLERANCE_Z;
        TOLLERANCE_XY = SMALL_TOLLERANCE_XY;
    } else
    {
        TOLLERANCE_Z = LARGE_TOLLERANCE_Z;
        TOLLERANCE_XY = LARGE_TOLLERANCE_XY;

    }

    ros::spinOnce();
    vec3f_t dis2go(disX, disY, disZ);
//    ROS_WARN_STREAM("[STARTMOVETO] dist2go :" << dis2go.toString());
    if (state.at(STARTPOS_IS_USEFUL) == 1)
    {
        ROS_ERROR_STREAM("[STARTMOVE] state(STARTPOS_IS_USEFUL)  ERROR !!");
        state.at(STARTPOS_IS_USEFUL) = 0;
    }
    updatePosToIdeal();
    startPX4Pos = currentPX4Pos;
    state.at(STARTPOS_IS_USEFUL) = 1;
    setpointPosIdeal = currentPosIdeal + dis2go;
    state.at(STATE_DESTPOS_ARRIVED) = 0;

    /**
     * Check the Z
     */
    float tempSetPoint = setpointPosIdeal.z;
    setpointPosIdeal.z = constrainF(tempSetPoint, HEIGHT_MAX, HEIGHT_MIN);
    if (tempSetPoint != setpointPosIdeal.z)
    {
        ROS_WARN_STREAM("[MOVE] the Z_sp comes to" << tempSetPoint
                                                   << ", constrain to:" << setpointPosIdeal.z);
        dis2go.z = setpointPosIdeal.z - currentPosIdeal.z;
    }
}


void StartMoveTo(bool is_precise, vec3f_t target) {
    StartMoveBy(is_precise, target.x - currentPosIdeal.x, target.y - currentPosIdeal.y, target.z - currentPosIdeal.z);
}


void EndMove() {
    ros::Rate loop_rate(REFRESH_RATE);
    while (state.at(STATE_DESTPOS_ARRIVED) == 0 && ros::ok())
    {
        ros::spinOnce();
        updatePosToIdeal();
        if (fabsf(currentVel.x) < MAX_XY_ACC * REFRESH_PERIOD * 2 &&
            fabsf(currentVel.y) < MAX_XY_ACC * REFRESH_PERIOD * 2 &&
            fabsf(currentVel.z) < MAX_Z_ACC * REFRESH_PERIOD * 2)
        {
            state.at(STATE_DESTPOS_ARRIVED) = 1;
            break;
        }
        sendGentlyStopPos();
        loop_rate.sleep();
    }
    ros::Duration(0.5).sleep();
    currentVel.set(0, 0, 0);
    ros::spinOnce();
    if (state.at(STARTPOS_IS_USEFUL) == 0)
    {
        ROS_ERROR("[ENDMOVE] state(STARTPOS_IS_USEFUL)  ERROR !! ");
    }
    updatePosToIdeal();
    state.at(STARTPOS_IS_USEFUL) = 0;
    ROS_WARN_STREAM("[ ENDMOVE ]");
}

void PillarFromTo(int startPillar, int endPillar) {
    if (MoveMethodBetween[startPillar][endPillar] == 1)
    {
        MoveToZ(true, FLYING_OVER_PILLAR_HEIGHT);
        vec3f_t dis;
        dis = Targets[endPillar].postion - Targets[startPillar].postion;
        MoveBy(true, dis.x, dis.y);
        MoveToZ(true, PILLAR_AIM_HEIGHT);
    }
}


void TakeOff() {
    if (FAKE_UNMOVE_TEST_ENABLE)
    {
        ROS_WARN_STREAM("[CORE][FAKE] take off");
        ros::Duration(1).sleep();
        ROS_WARN_STREAM("[CORE][FAKE] take off complete");
        std_msgs::UInt8 status;
        status.data = 5;
        pub_FakeStatus.publish(status);
        vec3f_t pos{0, 0, 1};
        pub_FakePose.publish(pos.toPosCmd());
        currentPosIdeal = pos;
        return;
    }

    while (autonomy_status != 1 && ros::ok())
    {
        ros::Duration(1).sleep();
        ROS_INFO_THROTTLE(2, "[TAKE OFF] waiting for OFFBOARD");
        ros::spinOnce();
    }

    px4_autonomy::Takeoff cmd_tf;
    cmd_tf.take_off = 1;
    cmd_tf.header.stamp = ros::Time::now();
    pub_TakeOff.publish(cmd_tf);

    while (autonomy_status != 5 && ros::ok())
    {
        ros::Duration(0.2).sleep();
        ROS_INFO_THROTTLE(2, "[TAKE OFF] taking off...");
        ros::spinOnce();
    }

    currentPosIdeal.z = currentPX4Pos.z;
    state.at(STARTPOS_IS_USEFUL) = 0;


    Hover();

    ros::Duration(0.35).sleep();
    ROS_INFO_STREAM("[TAKE OFF]Take off complete");
}

void updatePosToIdeal() {
    if (state.at(STARTPOS_IS_USEFUL) == 1)
    {
        currentPosIdeal = currentPosIdeal + currentPX4Pos - startPX4Pos;
    }
    startPX4Pos = currentPX4Pos;
}

void Hover() {
    vec3f_t cmd(0, 0, 0);
    pub_Vel.publish(cmd.toVelCmd());
    ros::spinOnce();
}


void Land() {
    if (FAKE_UNMOVE_TEST_ENABLE)
    {
        ROS_WARN_STREAM("[CORE][FAKE] landing");
        ros::Duration(2).sleep();
        ROS_WARN_STREAM("[CORE][FAKE] land complete");
        currentPosIdeal.z = 0;
        pub_FakePose.publish(currentPosIdeal.toPosCmd());
        return;
    }
    while (autonomy_status != 4 && autonomy_status != 5 && ros::ok())
    {
        ros::Duration(1).sleep();
        ROS_INFO("[CORE] Ready to land, but status not 4 or 5!!");
        ros::spinOnce();
    }

    px4_autonomy::Takeoff cmd_tf;
    cmd_tf.take_off = 2;
    cmd_tf.header.stamp = ros::Time::now();
    pub_TakeOff.publish(cmd_tf);

    while (autonomy_status != 1 && ros::ok())
    {
        ros::Duration(0.2).sleep();
        ROS_INFO("[CORE] landing...");
        ros::spinOnce();
    }

    ROS_INFO("[CORE] landed");
}

void CB_PX4Pose(const px4_autonomy::Position &msg) {
    vec3f_t px4_raw, px4_raw_corrected;
    px4_raw = msg;
    px4_raw_corrected.set(px4_raw.y, -px4_raw.x, px4_raw.z);
    currentPX4Pos = px4_raw_corrected - originPX4Pos;
}

void CB_status(const std_msgs::UInt8 &msg) {
    autonomy_status = msg.data;
}

void
ROSPrintFirstFindTarget(const std::string &CBFName, const vec3f_t &poseFromCam,
                        const std::pair<size_t, float> &searchRes) {

}


int camera_lost_result_count = 0;

void CB_Camera(const pub_rospy::total::ConstPtr &msg) {
    if (state.at(STAGE_NUM) != 0 && state.at(STAGE_NUM) != 1 && state.at(STAGE_NUM) != 3)
        return;
    if (!msg->result.empty() && msg->result[0].label != 88)
    {
        int count = 0;
        float label;
        for (const auto &i : msg->result)
        {
            if (i.z < 2.5 && i.z > 0.05)
            {
                if (count == 0)
                {
                    label = i.label;
                    targetPos.x = i.z;
                    targetPos.y = -i.x;
                    targetPos.z = -i.y;
                    state.at(TARGET_DETECTED) = 1;
                    state.at(TARGETPOS_UPDATED) = 1;
                    camera_lost_result_count = 0;
                    count++;
                    if (int(label) == state.at(TARGET_CIRCLE_NUM))
                    {
                        break;
                    }
                } else if ((int) i.label == state.at(TARGET_CIRCLE_NUM))
                {
                    targetPos.x = i.z;
                    targetPos.y = -i.x;
                    targetPos.z = -i.y;
                    state.at(TARGETPOS_UPDATED) = 1;
                    state.at(TARGET_DETECTED) = 1;
                    camera_lost_result_count = 0;
                    break;
                }
            }
        }
    }
    if (state.at(TARGETPOS_UPDATED) == 0)
    {
        camera_lost_result_count++;
        if (camera_lost_result_count > 7)
            state.at(TARGET_DETECTED) = 0;
    }
}


void getParas(ros::NodeHandle &n) {
    n.getParam("/strategy2019/REFRESH_RATE", REFRESH_RATE);
    n.getParam("/strategy2019/BOARD_AIM_HEIGHT", BOARD_AIM_HEIGHT);
    n.getParam("/strategy2019/MAX_XY_ACC", MAX_XY_ACC);
    n.getParam("/strategy2019/MAX_XY_VEL", MAX_XY_VEL);
    n.getParam("/strategy2019/MAX_Z_ACC", MAX_Z_ACC);
    n.getParam("/strategy2019/MAX_Z_VEL", MAX_Z_VEL);
    n.getParam("/strategy2019/HEIGHT_MAX", HEIGHT_MAX);
    n.getParam("/strategy2019/HEIGHT_MAX", HEIGHT_MAX);
    std::vector<int> direc_temp;
    n.getParam("/strategy2019/circleDirection", direc_temp);

    for (int i = 0; i < direc_temp.size(); i++)
        circleDirection.push_back((SearchDirection) (direc_temp[i]));


    n.getParam("/strategy2019/LEFT_MOVING_MAX", LEFT_MOVING_MAX);
    n.getParam("/strategy2019/RIGHT_MOVING_MAX", RIGHT_MOVING_MAX);
    n.getParam("/strategy2019/LEFT_NET_BOUND", LEFT_NET_BOUND);
    n.getParam("/strategy2019/RIGHT_NET_BOUND", RIGHT_NET_BOUND);


    n.getParam("/strategy2019/LARGE_TOLLERANCE_Z", LARGE_TOLLERANCE_Z);
    n.getParam("/strategy2019/LARGE_TOLLERANCE_XY", LARGE_TOLLERANCE_XY);
    n.getParam("/strategy2019/SMALL_TOLLERANCE_Z", SMALL_TOLLERANCE_Z);
    n.getParam("/strategy2019/SMALL_TOLLERANCE_XY", SMALL_TOLLERANCE_XY);

    std::vector<float> bias_temp;
    n.getParam("/strategy2019/BIAS_TO_YELLOW_BOARD", bias_temp);
    BIAS_TO_YELLOW_BOARD = vec3f_t{bias_temp[0], bias_temp[1], bias_temp[2]};

    std::vector<float> bias_pillar_temp;
    n.getParam("/strategy2019/BIAS_TO_PILLAR", bias_pillar_temp);
    BIAS_TO_PILLAR = vec3f_t{bias_temp[0], bias_temp[1], bias_temp[2]};

    n.getParam("/strategy2019/BOARD_AIM_HEIGHT", BOARD_AIM_HEIGHT);
    n.getParam("/strategy2019/GO_THROUGH_CIRCLE_HEIGHT", GO_THROUGH_CIRCLE_HEIGHT);
    n.getParam("/strategy2019/GO_THROUGH_CIRCLE_DISTANCE", GO_THROUGH_CIRCLE_DISTANCE);
    n.getParam("/strategy2019/PILLAR_AIM_HEIGHT", PILLAR_AIM_HEIGHT);
    n.getParam("/strategy2019/FLYING_OVER_PILLAR_HEIGHT", FLYING_OVER_PILLAR_HEIGHT);


    REFRESH_PERIOD = 1.f / REFRESH_RATE;
    stateold = state;
}


void InitPlaces() {


    Targets.emplace_back(0, vec3f_t{0, 0, 0}, CIRCLE);

    Targets.emplace_back(1, vec3f_t{3, 0, 0}, CIRCLE, circleDirection[1]);
    Targets.emplace_back(2, vec3f_t{6, 0, 0}, CIRCLE, circleDirection[2]);
    Targets.emplace_back(3, vec3f_t{9, 0, 0}, CIRCLE, circleDirection[3]);
    Targets.emplace_back(4, vec3f_t{12, 0, 0}, CIRCLE, circleDirection[4]);
    Targets.emplace_back(5, vec3f_t{15, 0, 0}, CIRCLE, circleDirection[5]);
    Targets.emplace_back(6, vec3f_t{18, 0, 0}, CIRCLE, circleDirection[6]);

    Targets.emplace_back(7, vec3f_t{22, 1.6, 0}, PILLAR);
    Targets.emplace_back(8, vec3f_t{22, -0.4, 0}, PILLAR);
    Targets.emplace_back(9, vec3f_t{22, -2.4, 0}, PILLAR);
    Targets.emplace_back(10, vec3f_t{25, -2.5, 0}, PILLAR);
    Targets.emplace_back(11, vec3f_t{25, 0, 0}, PILLAR);
    Targets.emplace_back(12, vec3f_t{25, 2.5, 0}, PILLAR);
    Targets.emplace_back(13, vec3f_t{28.5, 2.4, 0}, PILLAR);
    Targets.emplace_back(14, vec3f_t{28.5, 0.4, 0}, PILLAR);
    Targets.emplace_back(15, vec3f_t{28.5, 1.6, 0}, PILLAR);

    for (int i = 0; i < 17; i++)
    {
        for (int j = 0; j < 17; j++)
        {
            if (i < 7 || j < 7)
                MoveMethodBetween[i][j] = -1;
            else
            {
                MoveMethodBetween[i][j] = 1;
            }
        }
    }


}

void CB_outputTimer(const ros::TimerEvent &e) {
    vec3f_t goal;
    goal = setpointPosIdeal - currentPosIdeal;
    ROS_INFO_STREAM(std::fixed << std::setprecision(2) << "NOW: " << currentPosIdeal.toString() << " DEST: "
                               << setpointPosIdeal.toString() << " MOVE forward: "
                               << goal.x << "\t<= " << goal.y << "\tup: " << goal.z << " PX4Pos: "
                               << currentPX4Pos.toString() << " velSP: " << currentVel.toString());

    if (state.at(TARGET_CIRCLE_NUM) != stateold.at(TARGET_CIRCLE_NUM))
        ROS_WARN_STREAM("TARGET_CIRCLE_NUM: " << state.at(TARGET_CIRCLE_NUM));

    if (state.at(STATE_DESTPOS_ARRIVED) != stateold.at(STATE_DESTPOS_ARRIVED))
        ROS_WARN_STREAM("STATE_DESTPOS_ARRIVED: " << state.at(STATE_DESTPOS_ARRIVED));

    if (state.at(STAGE_NUM) != stateold.at(STAGE_NUM))
        ROS_WARN_STREAM("STAGE_NUM: " << state.at(STAGE_NUM));

    if (state.at(STARTPOS_IS_USEFUL) != stateold.at(STARTPOS_IS_USEFUL))
        ROS_WARN_STREAM("STARTPOS_IS_USEFUL: " << state.at(STARTPOS_IS_USEFUL));

    if (state.at(TARGET_DETECTED) != stateold.at(TARGET_DETECTED))
        ROS_WARN_STREAM("TARGET_DETECTED: " << state.at(TARGET_DETECTED));

    stateold = state;
}


void CB_camSigTimer(const ros::TimerEvent &e) {
    std_msgs::UInt8 camcmd;
    camcmd.data = state.at(STAGE_NUM);
    pub_camCmd.publish(camcmd);
}


void sendAutonomyPos() {


    if (autonomy_status != 4 && autonomy_status != 5)
    {
        ROS_INFO("[MOVE] Ready to MOVE, but status is not 4 or 5!!");
        return;
    }

    vec3f_t deltapos = setpointPosIdeal - currentPosIdeal;

    vec3f_t deltapos_z, deltapos_para, deltapos_vert;
    vec3f_t vel_z, vel_para, vel_vert;
    vec3f_t nextvel_para, nextvel_vert, nextvel_z, nextpos_para, nextpos_vert, nextpos_z;

    vel_z = vec3f_t{0, 0, currentVel.z};
    deltapos_z = vec3f_t{0, 0, deltapos.z};

    vec3f_t deltapos_level = deltapos - deltapos_z;
    vec3f_t vel_level = currentVel - vel_z;
    vec3f_t deltaposlevelDirection;
    if (deltapos_level.len() > 0)
    {
        deltaposlevelDirection = deltapos_level * (1.0f / deltapos_level.len());
    } else
    {
        deltaposlevelDirection.set(0, 0, 0);
    }

    vel_para = deltaposlevelDirection * (vel_level.dot(deltaposlevelDirection));
    vel_vert = vel_level - vel_para;

    calNext(vel_z, deltapos_z, nextvel_z, nextpos_z, MAX_Z_ACC, MAX_Z_VEL);
    calNext(vel_para, deltapos_level, nextvel_para, nextpos_para, MAX_XY_ACC, MAX_XY_VEL);
    calNext(vel_vert, vec3f_t{0, 0, 0}, nextvel_vert, nextpos_vert, MAX_XY_ACC, MAX_XY_VEL);


    vec3f_t nextposSP = originPX4Pos + currentPX4Pos + nextpos_z + nextpos_para + nextpos_vert;

    vec3f_t nextposSP_inPX4coor;
    nextposSP_inPX4coor.set(-nextposSP.y, nextposSP.x, nextposSP.z);

    pub_Pose.publish(nextposSP_inPX4coor.toPosCmd());
    currentVel = nextvel_z + nextvel_para + nextvel_vert;

//    ROS_INFO_STREAM("nexsposSP:  " << nextposSP.toString());

    if (FAKE_UNMOVE_TEST_ENABLE)
    {
        pub_FakePose.publish(nextposSP.toPosCmd());
    }
}

void sendGentlyStopPos() {

    if (autonomy_status != 4 && autonomy_status != 5)
    {
        ROS_INFO("[sendGentlyStopPos] status is not 4 or 5!!");
        return;
    }

    vec3f_t deltapos_zero = {0, 0, 0};
    vec3f_t vel_z, vel_para, vel_vert;
    vec3f_t nextvel_level, nextvel_z, nextpos_level, nextpos_z;

    vel_z = vec3f_t{0, 0, currentVel.z};

    vec3f_t vel_level = currentVel - vel_z;

    calNext(vel_z, deltapos_zero, nextvel_z, nextpos_z, MAX_Z_ACC, MAX_Z_VEL);
    calNext(vel_level, deltapos_zero, nextvel_level, nextpos_level, MAX_XY_ACC, MAX_XY_VEL);


    vec3f_t nextposSP = currentPX4Pos + nextpos_z + nextpos_level;

    pub_Pose.publish(nextposSP.toPosCmd());
    currentVel = nextvel_z + nextvel_level;

    if (FAKE_UNMOVE_TEST_ENABLE)
    {
        pub_FakePose.publish(nextposSP.toPosCmd());
    }
}

void
calNext(const vec3f_t velToThisDirection, const vec3f_t disToThisDirection, vec3f_t &outputvel, vec3f_t &outputdis,
        float MAX_ACC, float MAX_VEL) {


    float vel, dis;
    vel = velToThisDirection.len();
    dis = disToThisDirection.len();

    if (dis < 0.5f * MAX_ACC * REFRESH_PERIOD * REFRESH_PERIOD)
    {
        if (vel < MAX_ACC * REFRESH_PERIOD)
        {
            outputdis = disToThisDirection;
            outputvel.set(0, 0, 0);
            return;
        } else
        {
            if (vel > MAX_ACC * REFRESH_PERIOD)
            {
                vec3f_t Direction = velToThisDirection * (1.0f / vel);
                outputdis = Direction * (vel * REFRESH_PERIOD - 0.5f * MAX_ACC * REFRESH_PERIOD * REFRESH_PERIOD);
                outputvel = Direction * (vel - REFRESH_PERIOD * MAX_ACC);
                return;
            } else
            {
                outputdis = disToThisDirection;
                outputvel.set(0, 0, 0);
                return;
            }
        }
    }

    vec3f_t Direction = disToThisDirection * (1.0f / dis);
    bool is_same_direction = velToThisDirection.dot(disToThisDirection) >= 0;

    if (vel > MAX_VEL)
    {
        outputdis = Direction * (vel * REFRESH_PERIOD - 0.5f * MAX_ACC * REFRESH_PERIOD * REFRESH_PERIOD);
        outputvel = Direction * (vel - REFRESH_PERIOD * MAX_ACC);
        return;
    }


    if (is_same_direction)
    {
        float dis1, dis2, dis3, dis4;
        dis1 = 0.5f * (vel + MAX_VEL) * (MAX_VEL - vel) / MAX_ACC;
        dis3 = 0.5f * MAX_VEL * MAX_VEL / MAX_ACC;
        dis4 = 0.5f * vel * vel / MAX_ACC;   // min distance to fly before stop.

        if (dis >= dis1 + dis3)
        {
            if (vel + MAX_ACC * REFRESH_PERIOD > MAX_VEL)
            {
                outputvel = Direction * MAX_VEL;
                outputdis = Direction * (dis1 + MAX_VEL * (REFRESH_PERIOD - (MAX_VEL - vel) / MAX_ACC));
                return;
            } else
            {
                outputdis = Direction * (vel * REFRESH_PERIOD + 0.5f * MAX_ACC * REFRESH_PERIOD * REFRESH_PERIOD);
                outputvel = Direction * (vel + REFRESH_PERIOD * MAX_ACC);
                return;
            }
        } else if (dis > dis4)
        {
            // (v1^2-v^2)/2a+v1^2/2a=dis   ==> 2v1^2=dis*2a+v^2   ==> v1 = sqrt{ (dis*2a+v^2)/2 }
            float v1 = sqrtf((dis * 2 * MAX_ACC + vel * vel) / 2.0f);
            float s1 = 0.5f * (v1 * v1 - vel * vel) / MAX_ACC;
            float t1 = (v1 - vel) / MAX_ACC;
            if (REFRESH_PERIOD > t1)
            {
                outputvel = Direction * MAX_VEL;
                outputdis = Direction * (s1 + v1 * (REFRESH_PERIOD - t1) -
                                         0.5f * MAX_ACC * (REFRESH_PERIOD - t1) * (REFRESH_PERIOD - t1));
                return;
            } else
            {
                outputdis = Direction * (vel * REFRESH_PERIOD + 0.5f * MAX_ACC * REFRESH_PERIOD * REFRESH_PERIOD);
                outputvel = Direction * (vel + REFRESH_PERIOD * MAX_ACC);
                return;
            }
        } else
        {
            outputdis = Direction * (vel * REFRESH_PERIOD - 0.5f * MAX_ACC * REFRESH_PERIOD * REFRESH_PERIOD);
            outputvel = Direction * (vel - REFRESH_PERIOD * MAX_ACC);
            return;
        }

    } else
    {
        outputdis = Direction * (vel * REFRESH_PERIOD - 0.5f * MAX_ACC * REFRESH_PERIOD * REFRESH_PERIOD);
        outputvel = Direction * (vel - REFRESH_PERIOD * MAX_ACC);
        return;
    }
}

void aimToApron() {

    ROS_INFO_STREAM("[AIMTOAPRON]");
    if (state.at(STAGE_NUM) != 0 && state.at(STAGE_NUM) != 3)
        ROS_ERROR("STAGE_NUM SHOULD BE 0 OR 3!!");


    ROS_INFO_STREAM("[ AIMTOAPRON END]");
}

void aimToCircle() {
    ROS_INFO_STREAM("[AIM TO CIRCLE] " << state.at(TARGET_CIRCLE_NUM));
    if (state.at(STAGE_NUM) != 1)
        ROS_ERROR("STAGE_NUM SHOULD BE 1 !!");

    ros::Rate loop_rate(REFRESH_RATE);

    vec3f_t offset;

    if (state.at(TARGETPOS_UPDATED) == 1)
    {
        offset = targetPos + BIAS_TO_YELLOW_BOARD;
        state.at(TARGETPOS_UPDATED) = 0;
    }
    StartMoveBy(true, offset.x, offset.y, offset.z);


    while (state.at(STATE_DESTPOS_ARRIVED) == 0 && ros::ok())
    {
        ros::spinOnce();
        updatePosToIdeal();
        if (state.at(TARGETPOS_UPDATED) == 1)
        {
            offset = targetPos + BIAS_TO_YELLOW_BOARD;
            state.at(TARGETPOS_UPDATED) = 0;
            setpointPosIdeal = currentPosIdeal + offset;
        }
        vec3f_t error = setpointPosIdeal - currentPosIdeal;
//        ROS_INFO_STREAM_DELAYED_THROTTLE(2, "dis2adjust: " << error.toString());
        if (fabsf(error.x) < TOLLERANCE_XY && fabsf(error.y) < TOLLERANCE_XY && fabsf(error.z) < TOLLERANCE_Z)
        {
            state.at(STATE_DESTPOS_ARRIVED) = 1;
            break;
        }
        if (state.at(TARGET_DETECTED) == 0)
        {
            EndMove();
            ROS_ERROR_STREAM("[AIM TO CIRCLE " << state.at(TARGET_CIRCLE_NUM) << " LOST! ]");
            return;
        }
        sendAutonomyPos();

        loop_rate.sleep();
    }
    EndMove();

    ROS_INFO_STREAM("[AIM TO CIRCLE " << state.at(TARGET_CIRCLE_NUM) << " COMLETED]");
}


void aimToPillar() {
    ROS_WARN_STREAM("[AIM TO Pillar] " << state.at(TARGET_CIRCLE_NUM));
    if (state.at(STAGE_NUM) != 2)
        ROS_ERROR("STAGE_NUM SHOULD BE 2 !!");

    ros::Rate loop_rate(REFRESH_RATE);

    vec3f_t offset;

    if (state.at(TARGETPOS_UPDATED) == 1)
    {
        offset = targetPos + BIAS_TO_PILLAR;
        state.at(TARGETPOS_UPDATED) = 0;
    }
    StartMoveBy(true, offset.x, offset.y);


    while (state.at(STATE_DESTPOS_ARRIVED) == 0 && ros::ok())
    {
        ros::spinOnce();
        updatePosToIdeal();
        if (state.at(TARGETPOS_UPDATED) == 1)
        {
            offset = targetPos + BIAS_TO_PILLAR;
            state.at(TARGETPOS_UPDATED) = 0;
            setpointPosIdeal = currentPosIdeal + offset;
        }
        vec3f_t error = setpointPosIdeal - currentPosIdeal;
//        ROS_INFO_STREAM_DELAYED_THROTTLE(2, "dis2adjust: " << error.toString());
        if (fabsf(error.x) < TOLLERANCE_XY && fabsf(error.y) < TOLLERANCE_XY && fabsf(error.z) < TOLLERANCE_Z)
        {
            state.at(STATE_DESTPOS_ARRIVED) = 1;
            break;
        }
        if (state.at(TARGET_DETECTED) == 0)
        {
            EndMove();
            ROS_WARN_STREAM("[AIM TO PILLAR " << state.at(TARGET_CIRCLE_NUM) << " LOST! ]");
            return;
        }
        sendAutonomyPos();

        loop_rate.sleep();
    }
    EndMove();

    ROS_WARN_STREAM("[AIM TO PILLAR " << state.at(TARGET_CIRCLE_NUM) << " COMLETED]");
}


void CB_Sonar(const geometry_msgs::PointConstPtr &msg) {
    float leftdis, rightdis;
    leftdis = msg->x;
    rightdis = msg->y;

    if (state.at(SONAR_USEFUL) == 1)
    {
        if (setpointPosIdeal.y > 0)
        {
            if (leftdis > 0.01 && leftdis < 2)
                currentPosIdeal.y = LEFT_NET_BOUND - leftdis;
        } else
        {
            if (rightdis > 0.01 && rightdis < 2)
                currentPosIdeal.y = RIGHT_NET_BOUND + rightdis;
        }
    }

//    if (fabsf(currentPosIdeal.y - LEFT_MOVING_MAX) < 1.5)
//    {
//        if (leftdis > 0)
//        {
//            currentPosIdeal.y = LEFT_MOVING_MAX - leftdis;
//        } else
//        {
//            if (fabsf(setpointPosIdeal.y - LEFT_MOVING_MAX) < 1.5)
//            {
//                currentPosIdeal.y = LEFT_MOVING_MAX - 1.5f;
//            }
//        }
//    } else if (fabsf(currentPosIdeal.y - RIGHT_BOUND) < 1.5)
//    {
//        if (rightdis > 0)
//        {
//            currentPosIdeal.y = RIGHT_BOUND + rightdis;
//        } else
//        {
//            if (fabsf(setpointPosIdeal.y - RIGHT_BOUND) < 1.5)
//            {
//                currentPosIdeal.y = RIGHT_BOUND - 1.5f;
//            }
//        }
//    }
}

void CB_pillar(const std_msgs::Float32MultiArray &msg) {
    if (state.at(STAGE_NUM) != 2)
        return;


    std::vector<float> temp;
    temp = msg.data;
    if (temp[0] != 0)
    {
        state.at(TARGET_DETECTED) = 1;
        targetPos.x = temp[3];
        targetPos.y = temp[1];
        targetPos.z = -temp[2];
    } else{
        state.at(TARGET_DETECTED) = 0;
    }

}