/**
 * socket-task -- AIStatefulTask submodule - asynchronous (TLS) sockets support.
 *
 * @file
 * @brief Declaration of class AIEndPoint.
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

#pragma once

#include "resolver-task/AddrInfoLookup.h"
#include "evio/SocketAddress.h"
#include "resolver-task/GetAddrInfo.h"
#include <memory>

class AIEndPoint
{
 private:
  std::shared_ptr<resolver::AddrInfoLookup> m_cached;   // To be resolved (possibly already resolved) hostname.
  evio::SocketAddress m_address;                        // A single end point, only valid when m_cached is empty.
  union
  {                                                     // Iterator over the list of addresses, or null when we reached the end. Only valid after calling reset().
    struct addrinfo const* m_addrinfo;                  // Valid when m_cached is not empty.
    struct sockaddr const* m_sockaddr;                  // Valid when m_cached is empty.
  };

 public:
  // Construct empty AIEndPoint for assignment.
  AIEndPoint() = default;
  ~AIEndPoint() { }

  // AIEndPoint takes the same arguments as DnsResolver::getaddrinfo().
  template<typename S1, typename... Args, typename = typename std::enable_if<
    std::is_same<S1, std::string>::value || std::is_convertible<S1, std::string>::value>::type>
  AIEndPoint(S1&& node, Args... port_or_service_and_optional_hints)
    : m_cached(resolver::DnsResolver::instance().getaddrinfo(std::forward<std::string>(node), port_or_service_and_optional_hints...)) { }

  // Specify the exact end point by IP#, family and port.
  AIEndPoint(evio::SocketAddress const& address) : m_address(address) { }

  AIEndPoint(AIEndPoint const& orig) = default;
  AIEndPoint(AIEndPoint&& from) = default;
  AIEndPoint& operator=(AIEndPoint const& orig) { m_cached = orig.m_cached; m_address = orig.m_address; m_addrinfo = orig.m_addrinfo; return *this; }
  AIEndPoint& operator=(AIEndPoint&& from) { m_cached = std::move(from.m_cached); m_address = std::move(from.m_address); m_addrinfo = from.m_addrinfo; return *this; }

  bool is_ready() const { return !m_cached || m_cached->is_ready(); }
  bool used_dns() const { return !!m_cached; }

  // Reset the union iterator.
  bool reset();

  // Return the current address; only call this when the last call to reset() or next() returned true.
  evio::SocketAddress current() const;

  // Advance to the next address, if any.
  bool next();

  // Return the to be resolved (possibly already resolved) hostname.
  std::string hostname() const { return m_cached->hostname(); }

  // Return the port, if any was set.
  uint16_t port() const { return m_cached->get_port(); }

  // Create (if still empty) and run a GetAddrInfo task, calling cont() on the parent when ready.
  void run(boost::intrusive_ptr<task::GetAddrInfo>& task, AIStatefulTask* parent, AIStatefulTask::condition_type conditions COMMA_CWDEBUG_ONLY(bool debug_resolver));

  // Support writing to an ostream.
  friend std::ostream& operator<<(std::ostream& os, AIEndPoint const& end_point);
};
