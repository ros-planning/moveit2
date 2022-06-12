/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
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
 *   * Neither the name of Willow Garage nor the names of its
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

/* Author: Paul Gesel */

#pragma once

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <memory>

namespace node_interface
{
    struct NodeInterface
    {
//        template <typename T>
//        NodeInterface(T&& obj) : node_handle_(std::make_shared<Wrapper<T>>(std::forward<T>(obj))) {
//            static_assert(!std::is_base_of_v<T, std::shared_ptr<rclcpp_lifecycle::LifecycleNode>>
//                          || !std::is_base_of_v<T, std::shared_ptr<rclcpp::Node>>,
//                          "Node interface can only wrap rclcpp_lifecycle::LifecycleNode or rclcpp_lifecycle::LifecycleNode shared pointers");
//        }
        bool has_lifecycle_node_;

        NodeInterface(const std::shared_ptr<rclcpp::Node> & obj) :
            node_handle_(std::make_shared<Wrapper<std::shared_ptr<rclcpp::Node>>>(obj)) {
            has_lifecycle_node_ = false;
        }

        NodeInterface(const std::shared_ptr<rclcpp_lifecycle::LifecycleNode> & obj) :
                node_handle_(std::make_shared<Wrapper<std::shared_ptr<rclcpp_lifecycle::LifecycleNode>>>(obj)) {
                has_lifecycle_node_ = true;
        }

        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr get_node_base_interface() const {
            return node_handle_->get_node_base_interface();
        }

        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr get_node_topics_interface() const {
            return node_handle_->get_node_topics_interface();
        }

        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr get_node_parameters_interface() const{
            return node_handle_->get_node_parameters_interface();
        }

        struct  NodeBase
        {
            virtual rclcpp::node_interfaces::NodeBaseInterface::SharedPtr get_node_base_interface() const = 0;

            virtual rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr get_node_topics_interface() const = 0;

            virtual  rclcpp::node_interfaces::NodeParametersInterface::SharedPtr get_node_parameters_interface() const = 0;

            virtual std::shared_ptr<rclcpp::Node> get_rcl_node() const = 0;

            virtual ~NodeBase() {}
        };

        std::shared_ptr<rclcpp::Node> get_rcl_node() const
        {
            return node_handle_->get_rcl_node();
        }


        template<typename T>
        struct Wrapper : public NodeBase
        {

            Wrapper(const std::shared_ptr<rclcpp::Node>& t) : wrapped_node_(t) {
                rcl_node = t;
            }

            Wrapper(const std::shared_ptr<rclcpp_lifecycle::LifecycleNode>& t) : wrapped_node_(t) {}

           rclcpp::node_interfaces::NodeBaseInterface::SharedPtr get_node_base_interface() const override{
               return wrapped_node_->get_node_base_interface();
            }

            rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr get_node_topics_interface() const override{
                return wrapped_node_->get_node_topics_interface();
            }

            rclcpp::node_interfaces::NodeParametersInterface::SharedPtr get_node_parameters_interface() const override{
                return wrapped_node_->get_node_parameters_interface();
                }

            std::shared_ptr<rclcpp::Node> get_rcl_node() const override
            {
                return rcl_node;// std::dynamic_pointer_cast<std::shared_ptr<rclcpp::Node>>(wrapped_node_);
            }

            T wrapped_node_;
            std::shared_ptr<rclcpp::Node> rcl_node;
        };

        std::shared_ptr<NodeBase> node_handle_;
    };


    using NodeInterfaceSharedPtr = std::shared_ptr<NodeInterface>;

}  // namespace node_interface
