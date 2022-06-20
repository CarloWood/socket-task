/**
 * socket-task -- AIStatefulTask submodule - asynchronous (TLS) sockets support.
 *
 * @file
 * @brief Connect to an end point. Declaration of class ConnectToEndPoint.
 *
 * @Copyright (C) 2018  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of socket-task.
 *
 * Socket-task is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Socket-task is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with socket-task.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "statefultask/AIStatefulTask.h"
#include "resolver-task/GetAddrInfo.h"
#include "evio/Socket.h"
#include "AIEndPoint.h"
#include "debug.h"

namespace task {

/**
 * The socket task.
 *
 * Before calling @link group_run run()@endlink, call set_end_point() to pass needed parameters.
 */
class ConnectToEndPoint : public AIStatefulTask
{
 protected:
  /// The base class of this task.
  using direct_base_type = AIStatefulTask;

  /// The different states of the stateful task.
  enum connect_to_end_point_state_type {
    ConnectToEndPoint_start = direct_base_type::state_end,
    ConnectToEndPoint_connect_begin,
    ConnectToEndPoint_connect,
    ConnectToEndPoint_connect_wait,
    ConnectToEndPoint_connected,
    ConnectToEndPoint_connect_failed,
    ConnectToEndPoint_done
  };

  AIEndPoint m_end_point;                                       // The end point to connect to, set with set_end_point.
  boost::intrusive_ptr<task::GetAddrInfo> m_get_addr_info;
  boost::intrusive_ptr<evio::Socket> m_socket;                  // The socket to use for the connection, set with set_socket.
  bool m_connect_success;
  bool m_clean_disconnect;
  std::function<void (bool)> m_connected_cb;                    // Called upon successful connect, with parameter true; or upon permanent failure, with parameter false.

 public:
  /// One beyond the largest state of this task.
  static constexpr state_type state_end = ConnectToEndPoint_done + 1;

  /// Construct an ConnectToEndPoint object.
  ConnectToEndPoint(CWDEBUG_ONLY(bool debug = false)) : AIStatefulTask(CWDEBUG_ONLY(debug))
    { DoutEntering(dc::statefultask(mSMDebug), "ConnectToEndPoint() [" << (void*)this << "]"); }

  /// Set socket.
  void set_socket(boost::intrusive_ptr<evio::Socket>&& socket);

  boost::intrusive_ptr<evio::Socket> get_socket() const { return m_socket; }

  /**
   * Set the end point to connect to.
   *
   * @param end_point The internet end point that should be connected to.
   */
  void set_end_point(AIEndPoint&& end_point) { m_end_point = std::move(end_point); }

  AIEndPoint const& get_end_point() const { return m_end_point; }

  void on_connected(std::function<void (bool)> connected_cb) { m_connected_cb = std::move(connected_cb); }

 protected:
  /// Call finish() (or abort()), not delete.
  ~ConnectToEndPoint() { DoutEntering(dc::statefultask(mSMDebug), "~ConnectToEndPoint() [" << (void*)this << "]"); }

  /// Attempt a connect to address. Returning false is a failure, otherwise this function should result in a single call to connect_result.
  bool connect(evio::SocketAddress const& address);

  /// Implementation of virtual functions of AIStatefulTask.
  char const* state_str_impl(state_type run_state) const override;
  char const* task_name_impl() const override;

  /// Handle mRunState.
  void multiplex_impl(state_type run_state) override;

 public:
  // Called by the Socket when the socket is connected.
  void connected_cb(int& allow_deletion_count, bool success);
  void disconnected_cb(int& allow_deletion_count, bool success);
};

} // namespace task
