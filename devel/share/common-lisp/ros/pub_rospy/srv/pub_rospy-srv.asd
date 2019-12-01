
(cl:in-package :asdf)

(defsystem "pub_rospy-srv"
  :depends-on (:roslisp-msg-protocol :roslisp-utils )
  :components ((:file "_package")
    (:file "srv1" :depends-on ("_package_srv1"))
    (:file "_package_srv1" :depends-on ("_package"))
  ))