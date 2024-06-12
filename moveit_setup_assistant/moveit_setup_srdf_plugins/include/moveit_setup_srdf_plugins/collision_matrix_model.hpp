/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2016, CITEC, Bielefeld University
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

/* Author: Robert Haschke */

#pragma once

#include <QAbstractTableModel>
#include <srdfdom/srdf_writer.h>

#ifndef Q_MOC_RUN
#include <moveit_setup_srdf_plugins/compute_default_collisions.hpp>
#include <moveit_setup_srdf_plugins/default_collisions.hpp>
#endif

#include <QItemSelection>
namespace moveit_setup
{
namespace srdf_setup
{
class CollisionMatrixModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  CollisionMatrixModel(DefaultCollisions& default_collisions, const std::vector<std::string>& names,
                       QObject* parent = nullptr);
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  // for editing
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool setData(const QModelIndex& /*index*/, const QVariant& value, int role) override;
  void setEnabled(const QItemSelection& selection, bool value);
  void setEnabled(const QModelIndexList& indexes, bool value);

public Q_SLOTS:
  void setFilterRegExp(const QString& filter);

private:
  DefaultCollisions& default_collisions_;
  const std::vector<std::string> std_names_;  // names of links
  QList<QString> q_names_;                    // names of links
  QList<int> visual_to_index_;                // map from visual index to actual index
};
}  // namespace srdf_setup
}  // namespace moveit_setup
