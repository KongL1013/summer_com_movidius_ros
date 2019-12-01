cmake_minimum_required(VERSION 2.8.3)
project(pub_rospy)


find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  rospy
  sensor_msgs
  std_msgs
  message_generation
)

## Generate messages in the 'msg' folder
 add_message_files(
   FILES
   pubmsg.msg
   total.msg
 )


## Generate added messages and services with any dependencies listed here
generate_messages(
   DEPENDENCIES
   std_msgs
)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES pub_rospy
   #CATKIN_DEPENDS message_runtime 
#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  #include
  ${catkin_INCLUDE_DIRS}
)


#add_executable(get src/get.cpp)
#add_dependencies(get ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

#target_link_libraries(BGR_sample ${OpenCV_LIBS})
#target_link_libraries(BGR_sample ${catkin_LIBRARIES})
#target_link_libraries(get
#        ${catkin_LIBRARIES}
        #${OpenCV_LIBS}
#        )


