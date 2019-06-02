/**
 * @file
 * @brief Implementation of ConnectToEndPoint.
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

#include "sys.h"
#include "ConnectToEndPoint.h"

namespace task {

//static
void ConnectToEndPoint::s_connected(evio::Socket* socket, bool success)
{
  static_cast<ConnectToEndPoint*>(socket->VT_ptr->_input_user_data)->connect_result(success);
}

void ConnectToEndPoint::set_socket(boost::intrusive_ptr<evio::Socket>&& socket)
{
  m_socket = std::move(socket);
  // Dynamically override Socket::connected.
  m_socket_VT = std::shared_ptr<evio::Socket::VT_type>(m_socket->clone_VT());
  m_socket_VT->_input_user_data = this; // Abuse the virtual table by also storing a pointer to this.
  m_socket_VT->_connected = ConnectToEndPoint::s_connected;
}

char const* ConnectToEndPoint::state_str_impl(state_type run_state) const
{
  switch(run_state)
  {
    AI_CASE_RETURN(ConnectToEndPoint_start);
    AI_CASE_RETURN(ConnectToEndPoint_connect_begin);
    AI_CASE_RETURN(ConnectToEndPoint_connect);
    AI_CASE_RETURN(ConnectToEndPoint_connect_wait);
    AI_CASE_RETURN(ConnectToEndPoint_connected);
    AI_CASE_RETURN(ConnectToEndPoint_connect_failed);
    AI_CASE_RETURN(ConnectToEndPoint_done);
  }
  ASSERT(false);
  return "UNKNOWN STATE";
}

bool ConnectToEndPoint::connect(evio::SocketAddress const& address)
{
  return m_socket->connect(address);
}

void ConnectToEndPoint::connect_result(bool success)
{
  m_connect_success = success;
  signal(2);
}

// Socket
void ConnectToEndPoint::multiplex_impl(state_type run_state)
{
  switch (run_state)
  {
    case ConnectToEndPoint_start:
      // Do hostname lookup, if necessary.
      m_end_point.run(m_get_addr_info, this, 1 COMMA_DEBUG_ONLY(mSMDebug));
      // Wait until m_end_point becomes usable, then continue at state begin.
      set_state(ConnectToEndPoint_connect_begin);
      wait(1);
      break;
    case ConnectToEndPoint_connect_begin:
      // Try to connect to any address of m_end_point until one succeeds.
      if (!m_end_point.reset())  // Can this even happen?
      {
        abort();
        break;
      }
      set_state(ConnectToEndPoint_connect);
      /* FALL-THROUGH */
    case ConnectToEndPoint_connect:
    {
      evio::SocketAddress address = m_end_point.current();
      if (address.is_unspecified())
      {
        // Could not connect to any IP address.
        abort();
        break;
      }
      if (!connect(address))
      {
        set_state(ConnectToEndPoint_connect_failed);
        break;
      }
      // Wait for connect_result to be called.
      set_state(ConnectToEndPoint_connect_wait);
      wait(2);
      break;
    }
    case ConnectToEndPoint_connect_wait:
      if (!m_connect_success)
      {
        set_state(ConnectToEndPoint_connect_failed);
        break;
      }
      set_state(ConnectToEndPoint_connected);
      /* FALL-THROUGH */
    case ConnectToEndPoint_connected:
      set_state(ConnectToEndPoint_done);
      // Wait till connection is terminated.
      wait(4);
      break;
    case ConnectToEndPoint_connect_failed:
      // Advance to the next address, if any.
      if (!m_end_point.next())
      {
        Dout(dc::statefultask(mSMDebug), "None of the possible addresses succeeded to connect. Aborting.");
        abort();
        break;
      }
      set_state(ConnectToEndPoint_connect);
      break;
    case ConnectToEndPoint_done:
      finish();
      break;
  }
}

} // namespace task
