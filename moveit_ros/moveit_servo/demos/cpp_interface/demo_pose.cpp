/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2023, PickNik Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/*      Title     : demo_pose.cpp
 *      Project   : moveit_servo
 *      Created   : 06/07/2023
 *      Author    : V Mohammed Ibrahim
 *      Description : Example of controlling a robot through pose commands via the C++ API.
 */

#include <atomic>
#include <chrono>
#include <moveit_servo/servo.hpp>
#include <moveit_servo/utils/common.hpp>
#include <mutex>
#include <rclcpp/rclcpp.hpp>

using namespace moveit_servo;

namespace
{
const rclcpp::Logger LOGGER = rclcpp::get_logger("moveit_servo.pose_demo");
}

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  // The servo object expects to get a ROS node.
  auto demo_node = std::make_shared<rclcpp::Node>("moveit_servo_demo");

  // Get the servo parameters.
  std::string param_namespace = "moveit_servo";
  auto servo_param_listener = std::make_shared<const servo::ParamListener>(demo_node, param_namespace);
  auto servo_params = servo_param_listener->get_params();

  // The publisher to send trajectory message to the robot controller.
  rclcpp::Publisher<trajectory_msgs::msg::JointTrajectory>::SharedPtr trajectory_outgoing_cmd_pub =
      demo_node->create_publisher<trajectory_msgs::msg::JointTrajectory>(servo_params.command_out_topic,
                                                                         rclcpp::SystemDefaultsQoS());

  // Create the servo object
  auto planning_scene_monitor = createPlanningSceneMonitor(demo_node, servo_params);
  auto servo = Servo(demo_node, servo_param_listener, planning_scene_monitor);

  // Wait for some time, so that the planning scene is loaded in rviz.
  // This is just for convenience, should not be used for sync in real application.
  std::this_thread::sleep_for(std::chrono::seconds(3));

  // For syncing pose tracking thread and main thread.
  std::mutex pose_guard;
  std::atomic<bool> stop_tracking = false;

  // Set the command type for servo.
  servo.expectedCommandType(CommandType::POSE);

  // The dynamically updated target pose.
  PoseCommand target_pose;
  target_pose.frame_id = servo_params.ee_frame;
  target_pose.pose = Eigen::Isometry3d::Identity();  // Target pose in ee frame (same as the ee frame pose)
  // Convert the pose to planning frame.
  target_pose = servo.toPlanningFrame(target_pose);

  // The pose tracking lambda that will be run in a separate thread.
  auto pose_tracker = [&]() {
    KinematicState joint_state;
    rclcpp::WallRate tracking_rate(1 / servo_params.publish_period);
    while (!stop_tracking && rclcpp::ok())
    {
      {
        std::lock_guard<std::mutex> pguard(pose_guard);
        joint_state = servo.getNextJointState(target_pose);
      }
      StatusCode status = servo.getStatus();
      if (status != StatusCode::INVALID)
        trajectory_outgoing_cmd_pub->publish(composeTrajectoryMessage(servo_params, joint_state));

      tracking_rate.sleep();
    }
  };

  // Pose tracking thread will exit upon reaching this pose.
  Eigen::Isometry3d terminal_pose = target_pose.pose;
  terminal_pose.translate(Eigen::Vector3d(0.0, 0.0, -0.1));

  std::thread tracker_thread(pose_tracker);
  tracker_thread.detach();

  const double delta = 0.002;
  rclcpp::WallRate command_rate(50);
  RCLCPP_INFO_STREAM(LOGGER, servo.getStatusMessage());

  while (!stop_tracking && rclcpp::ok())
  {
    {
      std::lock_guard<std::mutex> pguard(pose_guard);
      target_pose.pose = servo.getEndEffectorPose();
      stop_tracking = target_pose.pose.isApprox(
          terminal_pose, servo_params.pose_tracking.linear_tolerance);  // Dynamically update the target pose
      target_pose.pose.translate(Eigen::Vector3d(0.0, 0.0, -delta));
    }

    command_rate.sleep();
  }

  RCLCPP_INFO_STREAM(LOGGER, "REACHED : " << stop_tracking);
  stop_tracking = true;

  if (tracker_thread.joinable())
    tracker_thread.join();

  RCLCPP_INFO(LOGGER, "Exiting demo.");
  rclcpp::shutdown();
}
