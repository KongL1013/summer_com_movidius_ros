
(cl:in-package :asdf)

(defsystem "pub_rospy-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :std_msgs-msg
)
  :components ((:file "_package")
    (:file "pubmsg" :depends-on ("_package_pubmsg"))
    (:file "_package_pubmsg" :depends-on ("_package"))
    (:file "total" :depends-on ("_package_total"))
    (:file "_package_total" :depends-on ("_package"))
  ))