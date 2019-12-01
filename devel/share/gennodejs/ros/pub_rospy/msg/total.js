// Auto-generated. Do not edit!

// (in-package pub_rospy.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let pubmsg = require('./pubmsg.js');
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class total {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.result = null;
      this.header = null;
    }
    else {
      if (initObj.hasOwnProperty('result')) {
        this.result = initObj.result
      }
      else {
        this.result = [];
      }
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type total
    // Serialize message field [result]
    // Serialize the length for message field [result]
    bufferOffset = _serializer.uint32(obj.result.length, buffer, bufferOffset);
    obj.result.forEach((val) => {
      bufferOffset = pubmsg.serialize(val, buffer, bufferOffset);
    });
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type total
    let len;
    let data = new total(null);
    // Deserialize message field [result]
    // Deserialize array length for message field [result]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.result = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.result[i] = pubmsg.deserialize(buffer, bufferOffset)
    }
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += 20 * object.result.length;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    return length + 4;
  }

  static datatype() {
    // Returns string type for a message object
    return 'pub_rospy/total';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '0c72830414d9245ad5dfa266b3a9c646';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    pubmsg[] result
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
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new total(null);
    if (msg.result !== undefined) {
      resolved.result = new Array(msg.result.length);
      for (let i = 0; i < resolved.result.length; ++i) {
        resolved.result[i] = pubmsg.Resolve(msg.result[i]);
      }
    }
    else {
      resolved.result = []
    }

    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    return resolved;
    }
};

module.exports = total;
