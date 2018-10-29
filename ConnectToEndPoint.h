/**
 * @file
 * @brief Connect to an end point. Declaration of class ConnectToEndPoint.
 *
 * @Copyright (C) 2018  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "statefultask/AIStatefulTask.h"
#include "resolver-task/GetAddrInfo.h"
#include "AIEndPoint.h"
#include "debug.h"

namespace task {

/*!
 * @brief The socket task.
 *
 * Before calling @link group_run run()@endlink, call set_end_point() to pass needed parameters.
 */

class ConnectToEndPoint : public AIStatefulTask
{
 protected:
  //! The base class of this task.
  using direct_base_type = AIStatefulTask;

  //! The different states of the stateful task.
  enum connect_to_end_point_state_type {
    ConnectToEndPoint_start = direct_base_type::max_state,
    ConnectToEndPoint_ready,
    ConnectToEndPoint_done
  };

  AIEndPoint m_end_point;
  boost::intrusive_ptr<task::GetAddrInfo> m_get_addr_info;

 public:
  //! One beyond the largest state of this task.
  static state_type constexpr max_state = ConnectToEndPoint_done + 1;

  /*!
   * @brief Construct an ConnectToEndPoint object.
   */
  ConnectToEndPoint(DEBUG_ONLY(bool debug = false)) DEBUG_ONLY(: AIStatefulTask(debug))
    { DoutEntering(dc::statefultask(mSMDebug), "ConnectToEndPoint() [" << (void*)this << "]"); }

  /*!
   * @brief Set the end point to connect to.
   *
   * @param end_point The internet end point that should be connected to.
   */
  void set_end_point(AIEndPoint&& end_point) { m_end_point = std::move(end_point); }

  AIEndPoint const& get_end_point() const { return m_end_point; }

 protected:
  //! Call finish() (or abort()), not delete.
  ~ConnectToEndPoint() override { DoutEntering(dc::statefultask(mSMDebug), "~ConnectToEndPoint() [" << (void*)this << "]"); }

  //! Implemenation of state_str for run states.
  char const* state_str_impl(state_type run_state) const override;

  //! Handle mRunState.
  void multiplex_impl(state_type run_state) override;
};

} // namespace task
