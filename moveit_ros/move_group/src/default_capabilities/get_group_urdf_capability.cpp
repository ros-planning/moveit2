/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2024, PickNik Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of PickNik Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Sebastian Jahr */

#include "get_group_urdf_capability.h"

#include <moveit/moveit_cpp/moveit_cpp.h>
#include <moveit/move_group/capability_names.h>
#include <moveit/utils/logger.hpp>
#include <tinyxml2.h>

namespace move_group
{

namespace
{
rclcpp::Logger getLogger()
{
  return moveit::getLogger("GetUrdfService");
}
const auto JOINT_ELEMENT_CLOSING = std::string("</joint>");
const auto LINK_ELEMENT_CLOSING = std::string("</link>");
const auto ROBOT_ELEMENT_CLOSING = std::string("</robot>")
}  // namespace

GetUrdfService::GetUrdfService() : MoveGroupCapability("get_group_urdf")
{
}

void GetUrdfService::initialize()
{
  get_urdf_service_ = context_->moveit_cpp_->getNode()->create_service<moveit_msgs::srv::GetGroupUrdf>(
      GET_URDF_SERVICE_NAME,
      [this](const std::shared_ptr<moveit_msgs::srv::GetGroupUrdf::Request>& req,
             const std::shared_ptr<moveit_msgs::srv::GetGroupUrdf::Response>& res) {
        auto const subgroup = context_->moveit_cpp_->getRobotModel()->getJointModelGroup(req->group_name);
        // Check if group exists in loaded robot model
        if (!subgroup)
        {
          RCLCPP_ERROR(getLogger(), "Cannot create URDF because planning group %s does not exist",
                       req->group_name.c_str());
          res->success = false;
          return;
        }
        // Get robot description string
        auto full_urdf_string = std::string("");
        context_->moveit_cpp_->getNode()->get_parameter_or("robot_description", full_urdf_string, std::string(""));

        // Check if string is empty
        if (full_urdf_string.empty())
        {
          RCLCPP_ERROR(getLogger(), "Couldn't load the urdf from parameter server. Is the /robot_description parameter "
                                    "initialized?");
          res->success = false;
          return;
        }

        // Create subgroup urdf
        // Create header
        res->urdf_string = std::string("<?xml version=\"1.0\" ?>\n<robot name=\"") + req->group_name +
                           std::string("\" xmlns:xacro=\"http://ros.org/wiki/xacro\">");

        // Create links
        auto const link_names = subgroup->getLinkModelNames();
        for (const auto& link_name : link_names)
        {
          auto const start = full_urdf_string.find("<link name=\"" + link_name);
          auto substring = full_urdf_string.substr(start, full_urdf_string.size() - start);
          res->urdf_string += substring.substr(0, substring.find(LINK_ELEMENT_CLOSING) + LINK_ELEMENT_CLOSING.size());
        }
        // Create joints
        auto const joint_names = subgroup->getJointModelNames();
        for (const auto& joint_name : joint_names)
        {
          auto const start = full_urdf_string.find("<joint name=\"" + joint_name);
          auto substring = full_urdf_string.substr(start, full_urdf_string.size() - start);
          res->urdf_string += substring.substr(0, substring.find(JOINT_ELEMENT_CLOSING) + JOINT_ELEMENT_CLOSING.size());
        }
        // Create closing
        res->urdf_string += ROBOT_ELEMENT_CLOSING;

        // Validate xml file
        tinyxml2::XMLDocument group_urdf_xml;
        group_urdf_xml.Parse(full_urdf_string.c_str());
        if (group_urdf_xml.Error())
        {
          RCLCPP_ERROR(getLogger(), "Failed to create valid urdf. tinyxml returned '%s'", group_urdf_xml.ErrorStr());
          res->success = false;
          return;
        }
      }  // End of callback function
  );
}
}  // namespace move_group

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(move_group::GetUrdfService, move_group::MoveGroupCapability)