/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2020, PickNik LLC
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
 *   * Neither the name of PickNik LLC nor the names of its
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

/* Author: Sebastian Jahr
   Description: The hybrid planning manager component node that serves as the control unit of the whole architecture.
 */

#pragma once

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include <moveit_msgs/action/local_planner.hpp>
#include <moveit_msgs/action/global_planner.hpp>
#include <moveit_msgs/action/hybrid_planning.hpp>


namespace moveit
{
namespace hybrid_planning
{
// The possible hybrid planner states
enum class HybridPlanningState : int8_t
{
  ABORT = -1,
  UNKNOWN = 0,
  READY = 1,
  REQUEST_RECEIVED = 2,
  GLOBAL_PLANNING_ACTIVE = 3,
  GLOBAL_PLAN_READY = 4,
  LOCAL_PLANNING_ACTIVE = 5,
  FINISHED = 6
};

// Hybrid planning manager component node
class HybridPlanningManager : public rclcpp::Node
{
public:
  HybridPlanningManager(const rclcpp::NodeOptions& options);

private:
  HybridPlanningState state_;  // TODO: Add state publisher topic
  rclcpp::TimerBase::SharedPtr timer_;

  std::shared_ptr<rclcpp_action::ServerGoalHandle<moveit_msgs::action::HybridPlanning>> hybrid_planning_goal_handle_;
  std::shared_ptr<moveit_msgs::action::HybridPlanning_Feedback> hybrid_planning_progess_;

  bool global_planning_started_;
  bool local_planning_started_;

  bool global_planning_executed_;
  bool local_planning_executed_;

  bool abort_;

  // Planning request action clients
  rclcpp_action::Client<moveit_msgs::action::LocalPlanner>::SharedPtr local_planner_action_client_;
  rclcpp_action::Client<moveit_msgs::action::GlobalPlanner>::SharedPtr global_planner_action_client_;

  // Hybrid planning request action server
  rclcpp_action::Server<moveit_msgs::action::HybridPlanning>::SharedPtr hybrid_planning_request_server_;

  bool initialize();

  // Hybrid planning goal callback for hybrid planning request server
  void hybridPlanningRequestCallback(
      std::shared_ptr<rclcpp_action::ServerGoalHandle<moveit_msgs::action::HybridPlanning>> goal_handle);

  // Run the hybrid planning manager's internal state machine
  void hybridPlanningLoop();

  // Request global planning
  int planGlobalTrajectory();

  // Start local planning and execution
  int runLocalPlanner();
};
}  // namespace hybrid_planning
}  // namespace moveit