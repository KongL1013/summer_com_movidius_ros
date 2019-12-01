;; Auto-generated. Do not edit!


(when (boundp 'pub_rospy::total)
  (if (not (find-package "PUB_ROSPY"))
    (make-package "PUB_ROSPY"))
  (shadow 'total (find-package "PUB_ROSPY")))
(unless (find-package "PUB_ROSPY::TOTAL")
  (make-package "PUB_ROSPY::TOTAL"))

(in-package "ROS")
;;//! \htmlinclude total.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass pub_rospy::total
  :super ros::object
  :slots (_result _header ))

(defmethod pub_rospy::total
  (:init
   (&key
    ((:result __result) (let (r) (dotimes (i 0) (push (instance pub_rospy::pubmsg :init) r)) r))
    ((:header __header) (instance std_msgs::Header :init))
    )
   (send-super :init)
   (setq _result __result)
   (setq _header __header)
   self)
  (:result
   (&rest __result)
   (if (keywordp (car __result))
       (send* _result __result)
     (progn
       (if __result (setq _result (car __result)))
       _result)))
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:serialization-length
   ()
   (+
    ;; pub_rospy/pubmsg[] _result
    (apply #'+ (send-all _result :serialization-length)) 4
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; pub_rospy/pubmsg[] _result
     (write-long (length _result) s)
     (dolist (elem _result)
       (send elem :serialize s)
       )
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; pub_rospy/pubmsg[] _result
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _result (let (r) (dotimes (i n) (push (instance pub_rospy::pubmsg :init) r)) r))
     (dolist (elem- _result)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;;
   self)
  )

(setf (get pub_rospy::total :md5sum-) "0c72830414d9245ad5dfa266b3a9c646")
(setf (get pub_rospy::total :datatype-) "pub_rospy/total")
(setf (get pub_rospy::total :definition-)
      "pubmsg[] result
std_msgs/Header header

================================================================================
MSG: pub_rospy/pubmsg
float32 label
float32 confidence
float32 x
float32 y
float32 z

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
# 0: no frame
# 1: global frame
string frame_id

")



(provide :pub_rospy/total "0c72830414d9245ad5dfa266b3a9c646")


