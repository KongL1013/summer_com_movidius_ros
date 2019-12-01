# summer_com_movidius_ros
本文件上传的是ros接受realsense rgb和深度图，传入movidius识别后，以自定义消息发布信息。
主文件是：src/rospy_test/src/rs.py。
在src/pub_rospy/src 下有自定义消息的python发布和c++接收实例，可以参考使用，并包含有自定义的消息文件，可以重新编译，也可以在devel文件夹下找已经编译好的自定义消息python包，直接拷贝到自己的ros包下使用。
