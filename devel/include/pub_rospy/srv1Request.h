// Generated by gencpp from file pub_rospy/srv1Request.msg
// DO NOT EDIT!


#ifndef PUB_ROSPY_MESSAGE_SRV1REQUEST_H
#define PUB_ROSPY_MESSAGE_SRV1REQUEST_H


#include <string>
#include <vector>
#include <map>

#include <ros/types.h>
#include <ros/serialization.h>
#include <ros/builtin_message_traits.h>
#include <ros/message_operations.h>


namespace pub_rospy
{
template <class ContainerAllocator>
struct srv1Request_
{
  typedef srv1Request_<ContainerAllocator> Type;

  srv1Request_()
    : A(0)
    , B(0)
    , C(0)  {
    }
  srv1Request_(const ContainerAllocator& _alloc)
    : A(0)
    , B(0)
    , C(0)  {
  (void)_alloc;
    }



   typedef int32_t _A_type;
  _A_type A;

   typedef int32_t _B_type;
  _B_type B;

   typedef int32_t _C_type;
  _C_type C;





  typedef boost::shared_ptr< ::pub_rospy::srv1Request_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::pub_rospy::srv1Request_<ContainerAllocator> const> ConstPtr;

}; // struct srv1Request_

typedef ::pub_rospy::srv1Request_<std::allocator<void> > srv1Request;

typedef boost::shared_ptr< ::pub_rospy::srv1Request > srv1RequestPtr;
typedef boost::shared_ptr< ::pub_rospy::srv1Request const> srv1RequestConstPtr;

// constants requiring out of line definition



template<typename ContainerAllocator>
std::ostream& operator<<(std::ostream& s, const ::pub_rospy::srv1Request_<ContainerAllocator> & v)
{
ros::message_operations::Printer< ::pub_rospy::srv1Request_<ContainerAllocator> >::stream(s, "", v);
return s;
}

} // namespace pub_rospy

namespace ros
{
namespace message_traits
{



// BOOLTRAITS {'IsMessage': True, 'IsFixedSize': True, 'HasHeader': False}
// {'pub_rospy': ['/home/dd/rospy_rs2/src/pub_rospy/msg'], 'std_msgs': ['/opt/ros/kinetic/share/std_msgs/cmake/../msg']}

// !!!!!!!!!!! ['__class__', '__delattr__', '__dict__', '__dir__', '__doc__', '__eq__', '__format__', '__ge__', '__getattribute__', '__gt__', '__hash__', '__init__', '__init_subclass__', '__le__', '__lt__', '__module__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', '__weakref__', '_parsed_fields', 'constants', 'fields', 'full_name', 'has_header', 'header_present', 'names', 'package', 'parsed_fields', 'short_name', 'text', 'types']




template <class ContainerAllocator>
struct IsMessage< ::pub_rospy::srv1Request_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::pub_rospy::srv1Request_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::pub_rospy::srv1Request_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::pub_rospy::srv1Request_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct HasHeader< ::pub_rospy::srv1Request_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct HasHeader< ::pub_rospy::srv1Request_<ContainerAllocator> const>
  : FalseType
  { };


template<class ContainerAllocator>
struct MD5Sum< ::pub_rospy::srv1Request_<ContainerAllocator> >
{
  static const char* value()
  {
    return "e7a68ce4e0b75a9719b4950a7069c9d4";
  }

  static const char* value(const ::pub_rospy::srv1Request_<ContainerAllocator>&) { return value(); }
  static const uint64_t static_value1 = 0xe7a68ce4e0b75a97ULL;
  static const uint64_t static_value2 = 0x19b4950a7069c9d4ULL;
};

template<class ContainerAllocator>
struct DataType< ::pub_rospy::srv1Request_<ContainerAllocator> >
{
  static const char* value()
  {
    return "pub_rospy/srv1Request";
  }

  static const char* value(const ::pub_rospy::srv1Request_<ContainerAllocator>&) { return value(); }
};

template<class ContainerAllocator>
struct Definition< ::pub_rospy::srv1Request_<ContainerAllocator> >
{
  static const char* value()
  {
    return "int32 A\n\
int32 B\n\
int32 C\n\
";
  }

  static const char* value(const ::pub_rospy::srv1Request_<ContainerAllocator>&) { return value(); }
};

} // namespace message_traits
} // namespace ros

namespace ros
{
namespace serialization
{

  template<class ContainerAllocator> struct Serializer< ::pub_rospy::srv1Request_<ContainerAllocator> >
  {
    template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
    {
      stream.next(m.A);
      stream.next(m.B);
      stream.next(m.C);
    }

    ROS_DECLARE_ALLINONE_SERIALIZER
  }; // struct srv1Request_

} // namespace serialization
} // namespace ros

namespace ros
{
namespace message_operations
{

template<class ContainerAllocator>
struct Printer< ::pub_rospy::srv1Request_<ContainerAllocator> >
{
  template<typename Stream> static void stream(Stream& s, const std::string& indent, const ::pub_rospy::srv1Request_<ContainerAllocator>& v)
  {
    s << indent << "A: ";
    Printer<int32_t>::stream(s, indent + "  ", v.A);
    s << indent << "B: ";
    Printer<int32_t>::stream(s, indent + "  ", v.B);
    s << indent << "C: ";
    Printer<int32_t>::stream(s, indent + "  ", v.C);
  }
};

} // namespace message_operations
} // namespace ros

#endif // PUB_ROSPY_MESSAGE_SRV1REQUEST_H
