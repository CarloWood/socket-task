/**
 * socket-task -- AIStatefulTask submodule - asynchronous (TLS) sockets support.
 *
 * @file
 * @brief Definition of class AIEndPoint.
 *
 * @Copyright (C) 2019  Carlo Wood.
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

#include "sys.h"
#include "AIEndPoint.h"

std::ostream& operator<<(std::ostream& os, AIEndPoint const& end_point)
{
  if (end_point.m_cached)
    os << "{AddrInfoLookup:" << *end_point.m_cached;
  else
    os << "{SocketAddress:" << end_point.m_address;
  return os << '}';
}

void AIEndPoint::run(boost::intrusive_ptr<task::GetAddrInfo>& task, AIStatefulTask* parent, AIStatefulTask::condition_type condition COMMA_CWDEBUG_ONLY(bool debug_resolver))
{
  // Only call run() once, and pass it a default constructed boost::intrusive_ptr<task::GetAddrInfo>.
  ASSERT(!task);
  // Did we ask for a hostname lookup and did it not finish yet?
  if (m_cached && !m_cached->is_ready())
  {
    task = new task::GetAddrInfo(CWDEBUG_ONLY(debug_resolver));
    task->init(m_cached);
    task->run(resolver::DnsResolver::instance().get_handler(), [parent, condition](bool){ parent->signal(condition); });
    return;
  }
  // No need to run at all.
  parent->signal(condition);
}

bool AIEndPoint::reset()
{
  if (m_cached)
  {
    // Don't call reset until a call to run() caused a call to parent->signal(condition).
    ASSERT(m_cached->is_ready());
    if (!m_cached->success())
      return false;             // DNS lookup failed.
    m_addrinfo = m_cached->get_result();
    // For now I'm assuming this can't happen. If it does happen then the code below might be a workaround.
    ASSERT(m_addrinfo);
    if (!m_addrinfo)
    {
      Dout(dc::warning, "DNS lookup succeeded, but no IP numbers found (cached result for \"" << m_cached->hostname() << "\")!");
      // Return false will cause get_error() to be called, so set some error.
      m_cached->set_error_empty();
      return false;
    }
  }
  else
    m_sockaddr = m_address;
  return true;
}

evio::SocketAddress AIEndPoint::current() const
{
  // Only call this when the last call to reset() / next() returned true.
  return m_cached ? evio::SocketAddress(m_addrinfo->ai_addr, m_cached->get_port()) : evio::SocketAddress(m_sockaddr);
}

bool AIEndPoint::next()
{
  if (m_cached)
  {
    if ((m_addrinfo = m_addrinfo->ai_next))
      return true;
  }
  else
    m_sockaddr = nullptr;
  return false;
}
