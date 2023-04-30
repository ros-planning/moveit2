// Copyright 2022 PickNik Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the PickNik Inc. nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/* Author    : Andy Zelenak
   Desc      : Free functions. We keep them in a separate translation unit to reduce .o filesize
   Title     : utilities.h
   Project   : moveit_servo
*/

#pragma once

#include <control_msgs/msg/joint_jog.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <moveit/robot_model/joint_model_group.h>
#include <moveit/robot_state/robot_state.h>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_eigen/tf2_eigen.hpp>

#include <moveit_servo/status_codes.h>
#include <moveit/online_signal_smoothing/smoothing_base_class.h>
#include <moveit_servo_lib_parameters.hpp>

namespace moveit_servo
{
// Helper function for converting Eigen::Isometry3d to geometry_msgs/TransformStamped
geometry_msgs::msg::TransformStamped convertIsometryToTransform(const Eigen::Isometry3d& eigen_tf,
                                                                const std::string& parent_frame,
                                                                const std::string& child_frame);

/** \brief Possibly calculate a velocity scaling factor, due to proximity of
 * singularity and direction of motion
 * @param[in] joint_model_group   The MoveIt group
 * @param[in] commanded_twist     The commanded Cartesian twist
 * @param[in] svd                 A singular value decomposition of the Jacobian
 * @param[in] pseudo_inverse      The pseudo-inverse of the Jacobian
 * @param[in] hard_stop_singularity_threshold  Halt if condition(Jacobian) > hard_stop_singularity_threshold
 * @param[in] lower_singularity_threshold      Decelerate if condition(Jacobian) > lower_singularity_threshold
 * @param[in] leaving_singularity_threshold_multiplier      Allow faster motion away from singularity
 * @param[in, out] clock          A ROS clock, for logging
 * @param[in, out] current_state  The state of the robot. Used in internal calculations.
 * @param[out] status             Singularity status
 */
double velocityScalingFactorForSingularity(const moveit::core::JointModelGroup* joint_model_group,
                                           const Eigen::VectorXd& commanded_twist,
                                           const Eigen::JacobiSVD<Eigen::MatrixXd>& svd,
                                           const Eigen::MatrixXd& pseudo_inverse,
                                           const double hard_stop_singularity_threshold,
                                           const double lower_singularity_threshold,
                                           const double leaving_singularity_threshold_multiplier, rclcpp::Clock& clock,
                                           const moveit::core::RobotStatePtr& current_state, StatusCode& status);

/** \brief Joint-wise update of a sensor_msgs::msg::JointState with given delta's
 * Also filters and calculates the previous velocity
 * @param clock A ROS clock, for logging
 * @param publish_period The publishing rate for servo command
 * @param delta_theta Eigen vector of joint delta's
 * @param previous_joint_state The previous joint state
 * @param next_joint_state The joint state object which will hold the next joint state.
 * @param smoother The trajectory smoother to be used.
 * @return Returns false if there is a problem, true otherwise
 */
bool applyJointUpdate(rclcpp::Clock& clock, const double publish_period, const Eigen::ArrayXd& delta_theta,
                      const sensor_msgs::msg::JointState& previous_joint_state,
                      sensor_msgs::msg::JointState& next_joint_state,
                      pluginlib::UniquePtr<online_signal_smoothing::SmoothingBaseClass>& smoother);

/** \brief Converts a twist command from the command frame to the MoveGroup planning frame of the robot
 * @param cmd The twist command
 * @param servo_params The servo parameters
 * @param tf_moveit_to_robot_cmd_frame Transform from MoveIt planning frame to servoing command frame
 * @param tf_moveit_to_ee_frame Transform from MoveIt planning frame to end-effector frame
 * @param current_state The state of the robot
 */
void commandToMoveGroupFrame(geometry_msgs::msg::TwistStamped& cmd, const servo::Params& servo_params,
                             const Eigen::Isometry3d& tf_moveit_to_robot_cmd_frame,
                             const Eigen::Isometry3d& tf_moveit_to_ee_frame,
                             const moveit::core::RobotStatePtr current_state);

/** \brief Converts the delta_x (change in cartesian position) to a pose to be used with IK solver.
 * @param delta_x The change in cartesian position
 * @param ik_base_to_tip_frame The transform from base of the robot to its end-effector
 * @return Returns the resulting pose after applying delta_x
 */
geometry_msgs::msg::Pose deltaToPose(const Eigen::VectorXd delta_x, const Eigen::Isometry3d ik_base_to_tip_frame);
}  // namespace moveit_servo
