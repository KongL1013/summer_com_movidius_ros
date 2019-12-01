;; Auto-generated. Do not edit!


(when (boundp 'pub_rospy::pubmsg)
  (if (not (find-package "PUB_ROSPY"))
    (make-package "PUB_ROSPY"))
  (shadow 'pubmsg (find-package "PUB_ROSPY")))
(unless (find-package "PUB_ROSPY::PUBMSG")
  (make-package "PUB_ROSPY::PUBMSG"))

(in-package "ROS")
;;//! \htmlinclude pubmsg.msg.html


(defclass pub_rospy::pubmsg
  :super ros::object
  :slots (_label _confidence _x _y _z ))

(defmethod pub_rospy::pubmsg
  (:init
   (&key
    ((:label __label) 0.0)
    ((:confidence __confidence) 0.0)
    ((:x __x) 0.0)
    ((:y __y) 0.0)
    ((:z __z) 0.0)
    )
   (send-super :init)
   (setq _label (float __label))
   (setq _confidence (float __confidence))
   (setq _x (float __x))
   (setq _y (float __y))
   (setq _z (float __z))
   self)
  (:label
   (&optional __label)
   (if __label (setq _label __label)) _label)
  (:confidence
   (&optional __confidence)
   (if __confidence (setq _confidence __confidence)) _confidence)
  (:x
   (&optional __x)
   (if __x (setq _x __x)) _x)
  (:y
   (&optional __y)
   (if __y (setq _y __y)) _y)
  (:z
   (&optional __z)
   (if __z (setq _z __z)) _z)
  (:serialization-length
   ()
   (+
    ;; float32 _label
    4
    ;; float32 _confidence
    4
    ;; float32 _x
    4
    ;; float32 _y
    4
    ;; float32 _z
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float32 _label
       (sys::poke _label (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _confidence
       (sys::poke _confidence (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _x
       (sys::poke _x (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _y
       (sys::poke _y (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _z
       (sys::poke _z (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float32 _label
     (setq _label (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _confidence
     (setq _confidence (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _x
     (setq _x (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _y
     (setq _y (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _z
     (setq _z (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get pub_rospy::pubmsg :md5sum-) "d681388b870b81593f8e8878a8139340")
(setf (get pub_rospy::pubmsg :datatype-) "pub_rospy/pubmsg")
(setf (get pub_rospy::pubmsg :definition-)
      "float32 label
float32 confidence
float32 x
float32 y
float32 z

")



(provide :pub_rospy/pubmsg "d681388b870b81593f8e8878a8139340")


