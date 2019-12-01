; Auto-generated. Do not edit!


(cl:in-package pub_rospy-msg)


;//! \htmlinclude total.msg.html

(cl:defclass <total> (roslisp-msg-protocol:ros-message)
  ((result
    :reader result
    :initarg :result
    :type (cl:vector pub_rospy-msg:pubmsg)
   :initform (cl:make-array 0 :element-type 'pub_rospy-msg:pubmsg :initial-element (cl:make-instance 'pub_rospy-msg:pubmsg)))
   (header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header)))
)

(cl:defclass total (<total>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <total>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'total)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name pub_rospy-msg:<total> is deprecated: use pub_rospy-msg:total instead.")))

(cl:ensure-generic-function 'result-val :lambda-list '(m))
(cl:defmethod result-val ((m <total>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader pub_rospy-msg:result-val is deprecated.  Use pub_rospy-msg:result instead.")
  (result m))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <total>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader pub_rospy-msg:header-val is deprecated.  Use pub_rospy-msg:header instead.")
  (header m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <total>) ostream)
  "Serializes a message object of type '<total>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'result))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'result))
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <total>) istream)
  "Deserializes a message object of type '<total>"
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'result) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'result)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'pub_rospy-msg:pubmsg))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<total>)))
  "Returns string type for a message object of type '<total>"
  "pub_rospy/total")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'total)))
  "Returns string type for a message object of type 'total"
  "pub_rospy/total")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<total>)))
  "Returns md5sum for a message object of type '<total>"
  "0c72830414d9245ad5dfa266b3a9c646")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'total)))
  "Returns md5sum for a message object of type 'total"
  "0c72830414d9245ad5dfa266b3a9c646")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<total>)))
  "Returns full string definition for message of type '<total>"
  (cl:format cl:nil "pubmsg[] result~%std_msgs/Header header~%~%================================================================================~%MSG: pub_rospy/pubmsg~%float32 label~%float32 confidence~%float32 x~%float32 y~%float32 z~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%# 0: no frame~%# 1: global frame~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'total)))
  "Returns full string definition for message of type 'total"
  (cl:format cl:nil "pubmsg[] result~%std_msgs/Header header~%~%================================================================================~%MSG: pub_rospy/pubmsg~%float32 label~%float32 confidence~%float32 x~%float32 y~%float32 z~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%# 0: no frame~%# 1: global frame~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <total>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'result) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <total>))
  "Converts a ROS message object to a list"
  (cl:list 'total
    (cl:cons ':result (result msg))
    (cl:cons ':header (header msg))
))
