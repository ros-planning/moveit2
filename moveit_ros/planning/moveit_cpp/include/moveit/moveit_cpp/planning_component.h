/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2019, PickNik Inc.
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

/* Author: Henning Kayser
   Desc: API for planning and execution capabilities of a JointModelGroup */

#pragma once

#include <rclcpp/rclcpp.hpp>
#include <moveit/moveit_cpp/moveit_cpp.h>
#include <moveit/robot_state/robot_state.h>
#include <geometry_msgs/msg/pose_stamped.h>
#include <moveit/robot_state/conversions.h>
#include <moveit/utils/moveit_error_code.h>

namespace moveit_cpp
{
MOVEIT_CLASS_FORWARD(PlanningComponent);  // Defines PlanningComponentPtr, ConstPtr, WeakPtr... etc

class PlanningComponent
{
public:
  MOVEIT_STRUCT_FORWARD(PlanSolution);

  using MoveItErrorCode [[deprecated("Use moveit::core::MoveItErrorCode")]] = moveit::core::MoveItErrorCode;

  /// The representation of a plan solution
  struct PlanSolution
  {
    /// The full starting state used for planning
    moveit_msgs::msg::RobotState start_state;

    /// The trajectory of the robot (may not contain joints that are the same as for the start_state_)
    robot_trajectory::RobotTrajectoryPtr trajectory;

    /// Reason why the plan failed
    moveit::core::MoveItErrorCode error_code;

    explicit operator bool() const
    {
      return bool(error_code);
    }
  };

  /// Planner parameters provided with the MotionPlanRequest
  struct PlanRequestParameters
  {
    std::string planner_id;
    std::string planning_pipeline;
    int planning_attempts;
    double planning_time;
    double max_velocity_scaling_factor;
    double max_acceleration_scaling_factor;

    void load(const rclcpp::Node::SharedPtr& node)
    {
      std::string ns = "plan_request_params.";
      node->get_parameter_or(ns + "planner_id", planner_id, std::string(""));
      node->get_parameter_or(ns + "planning_pipeline", planning_pipeline, std::string(""));
      node->get_parameter_or(ns + "planning_time", planning_time, 1.0);
      node->get_parameter_or(ns + "planning_attempts", planning_attempts, 5);
      node->get_parameter_or(ns + "max_velocity_scaling_factor", max_velocity_scaling_factor, 1.0);
      node->get_parameter_or(ns + "max_acceleration_scaling_factor", max_acceleration_scaling_factor, 1.0);
    }
  };

  /** \brief Constructor */
  PlanningComponent(const std::string& group_name, const rclcpp::Node::SharedPtr& node);
  PlanningComponent(const std::string& group_name, const MoveItCppPtr& moveit_cpp);

  /**
   * @brief This class owns unique resources (e.g. action clients, threads) and its not very
   * meaningful to copy. Pass by references, move it, or simply create multiple instances where
   * required.
   */
  PlanningComponent(const PlanningComponent&) = delete;
  PlanningComponent& operator=(const PlanningComponent&) = delete;

  PlanningComponent(PlanningComponent&& other) = delete;
  PlanningComponent& operator=(PlanningComponent&& other) = delete;

  /** \brief Destructor */
  ~PlanningComponent() = default;

  /** \brief Get the name of the planning group */
  const std::string& getPlanningGroupName() const;

  /** \brief Get the names of the named robot states available as targets */
  const std::vector<std::string> getNamedTargetStates();

  /** \brief Get the joint values for targets specified by name */
  std::map<std::string, double> getNamedTargetStateValues(const std::string& name);

  /** \brief Specify the workspace bounding box.
       The box is specified in the planning frame (i.e. relative to the robot root link start position).
       This is useful when the planning group contains the root joint of the robot -- i.e. when planning motion for the
     robot relative to the world. */
  void setWorkspace(double minx, double miny, double minz, double maxx, double maxy, double maxz);

  /** \brief Remove the workspace bounding box from planning */
  void unsetWorkspace();

  /** \brief Get the considered start state if defined, otherwise get the current state */
  moveit::core::RobotStatePtr getStartState();

  /** \brief Set the robot state that should be considered as start state for planning */
  bool setStartState(const moveit::core::RobotState& start_state);
  /** \brief Set the named robot state that should be considered as start state for planning */
  bool setStartState(const std::string& named_state);

  /** \brief Set the start state for planning to be the current state of the robot */
  void setStartStateToCurrentState();

  /** \brief Set the goal constraints used for planning */
  bool setGoal(const std::vector<moveit_msgs::msg::Constraints>& goal_constraints);
  /** \brief Set the goal constraints generated from a target state */
  bool setGoal(const moveit::core::RobotState& goal_state);
  /** \brief Set the goal constraints generated from target pose and robot link */
  bool setGoal(const geometry_msgs::msg::PoseStamped& goal_pose, const std::string& link_name);
  /** \brief Set the goal constraints generated from a named target state */
  bool setGoal(const std::string& named_target);

  /** \brief Set the path constraints generated from a moveit msg Constraints */
  bool setPathConstraints(const moveit_msgs::msg::Constraints& path_constraints);

  /** \brief Set the trajectory constraints generated from a moveit msg Constraints */
  bool setTrajectoryConstraints(const moveit_msgs::msg::TrajectoryConstraints& trajectory_constraints);

  /** \brief Run a plan from start or current state to fulfill the last goal constraints provided by setGoal() using
   * default parameters. */
  PlanSolution plan();
  /** \brief Run a plan from start or current state to fulfill the last goal constraints provided by setGoal() using the
   * provided PlanRequestParameters. */
  PlanSolution plan(const PlanRequestParameters& parameters);

  /** \brief Execute the latest computed solution trajectory computed by plan(). By default this function terminates
   * after the execution is complete. The execution can be run in background by setting blocking to false. */
  bool execute(bool blocking = true);

  /** \brief Return the last plan solution*/
  const PlanSolutionPtr getLastPlanSolution();

private:
  // Core properties and instances
  rclcpp::Node::SharedPtr node_;
  MoveItCppPtr moveit_cpp_;
  const std::string group_name_;
  // The robot_model_ member variable of MoveItCpp class will manually free the joint_model_group_ resources
  const moveit::core::JointModelGroup* joint_model_group_;

  // Planning
  std::set<std::string> planning_pipeline_names_;
  // The start state used in the planning motion request
  moveit::core::RobotStatePtr considered_start_state_;
  std::vector<moveit_msgs::msg::Constraints> current_goal_constraints_;
  moveit_msgs::msg::Constraints current_path_constraints_;
  moveit_msgs::msg::TrajectoryConstraints current_trajectory_constraints_;
  PlanRequestParameters plan_request_parameters_;
  moveit_msgs::msg::WorkspaceParameters workspace_parameters_;
  bool workspace_parameters_set_ = false;
  PlanSolutionPtr last_plan_solution_;

  // common properties for goals
  // TODO(henningkayser): support goal tolerances
  // double goal_joint_tolerance_;
  // double goal_position_tolerance_;
  // double goal_orientation_tolerance_;
  // TODO(henningkayser): implement path/trajectory constraints
  // std::unique_ptr<moveit_msgs::msg::Constraints> path_constraints_;
  // std::unique_ptr<moveit_msgs::msg::TrajectoryConstraints> trajectory_constraints_;
};
}  // namespace moveit_cpp

namespace moveit
{
namespace planning_interface
{
using PlanningComponent [[deprecated("use moveit_cpp")]] = moveit_cpp::PlanningComponent;
[[deprecated("use moveit_cpp")]] MOVEIT_DECLARE_PTR(PlanningComponent, moveit_cpp::PlanningComponent);
}  // namespace planning_interface
}  // namespace moveit
