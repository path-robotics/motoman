/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2016, Delft Robotics Institute
 * Copyright (c) 2020, Southwest Research Institute
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  * Neither the name of the Southwest Research Institute, nor the names
 *  of its contributors may be used to endorse or promote products derived
 *  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "motoman_driver/io_relay.h"
#include <string>
#include <ros/ros.h>

namespace motoman
{
namespace io_relay
{

using industrial::shared_types::shared_int;

bool MotomanIORelay::init(int default_port)
{
  std::string ip;
  int port;

  ros::param::get("robot_ip_address", ip);
  // override port with ROS param, if available
  ros::param::param<int>("~port", port, default_port);
  ROS_DEBUG_NAMED("io.init", "Using port: %d (default was: %d)", port, default_port);

  // check for valid parameter values
  if (ip.empty())
  {
    ROS_ERROR_NAMED("io.init", "No valid robot IP address found.  Please set ROS 'robot_ip_address' param");
    return false;
  }

  char* ip_addr = strdup(ip.c_str());  // connection.init() requires "char*", not "const char*"
  ROS_DEBUG_NAMED("io.init", "I/O relay attempting to connect to: tcp://%s:%d", ip_addr, port);
  if (!default_tcp_connection_.init(ip_addr, port))
  {
    ROS_ERROR_NAMED("io.init", "Failed to initialize TcpClient");
    return false;
  }
  free(ip_addr);

  if (!default_tcp_connection_.makeConnect())
  {
    ROS_ERROR_NAMED("io.init", "Failed to connect");
    return false;
  }

  if (!io_ctrl_.init(&default_tcp_connection_))
  {
    ROS_ERROR_NAMED("io.init", "Failed to initialize MotomanIoCtrl");
    return false;
  }

  this->srv_read_single_io = this->node_.advertiseService("read_single_io",
      &MotomanIORelay::readSingleIoCB, this);
  this->srv_write_single_io = this->node_.advertiseService("write_single_io",
      &MotomanIORelay::writeSingleIoCB, this);

  return true;
}

// Service to read a single IO
bool MotomanIORelay::readSingleIoCB(
  motoman_msgs::ReadSingleIO::Request &req,
  motoman_msgs::ReadSingleIO::Response &res)
{
  shared_int io_val= -1;

  // send message and release mutex as soon as possible
  this->mutex_.lock();
  bool result = io_ctrl_.readSingleIO(req.address, io_val);
  this->mutex_.unlock();

  if (!result)
  {
    ROS_ERROR_NAMED("io.read", "Reading IO element %d failed", req.address);
    return false;
  }
  res.value = io_val;
  ROS_DEBUG_NAMED("io.read", "Element %d value: %d", req.address, io_val);

  return true;
}


// Service to write Single IO
bool MotomanIORelay::writeSingleIoCB(
  motoman_msgs::WriteSingleIO::Request &req,
  motoman_msgs::WriteSingleIO::Response &res)
{
  shared_int io_val= -1;

  // send message and release mutex as soon as possible
  this->mutex_.lock();
  bool result = io_ctrl_.writeSingleIO(req.address, req.value);
  this->mutex_.unlock();

  if (!result)
  {
    ROS_ERROR_NAMED("io.write", "Writing IO element %d failed", req.address);
    return false;
  }
  ROS_DEBUG_NAMED("io.write", "Element %d set to: %d", req.address, req.value);

  return true;
}

}  // namespace io_relay
}  // namespace motoman

