#pragma once

#include "resolver-task/AddrInfoLookup.h"
#include "evio/SocketAddress.h"
#include "resolver-task/GetAddrInfo.h"
#include <memory>

class AIEndPoint
{
 private:
  std::shared_ptr<resolver::AddrInfoLookup> m_cached;   // To be resolved (possibly already resolved) hostname.
  evio::SocketAddress m_address;                        // A single end point, only valid when m_cached is nul.

 public:
  // Construct empty AIEndPoint for assignment.
  AIEndPoint() = default;

  // AIEndPoint takes the same arguments as Resolver::getaddrinfo().
  template<typename S1, typename... Args, typename = typename std::enable_if<
    std::is_same<S1, std::string>::value || std::is_convertible<S1, std::string>::value>::type>
  AIEndPoint(S1&& node, Args... port_or_service_and_optional_hints)
    : m_cached(resolver::Resolver::instance().getaddrinfo(std::forward<std::string>(node), port_or_service_and_optional_hints...)) { }

  // Specify the exact end point by IP#, family and port.
  AIEndPoint(evio::SocketAddress const& address) : m_address(address) { }

  bool is_ready() const { return !m_cached || m_cached->is_ready(); }
  evio::SocketAddress get_address() const;

  // Create (if still empty) and run a GetAddrInfo task, calling cont() on the parent when ready.
  void run(boost::intrusive_ptr<task::GetAddrInfo>& task, AIStatefulTask* parent, AIStatefulTask::condition_type conditions COMMA_DEBUG_ONLY(bool debug_resolver));

  // Support writing to an ostream.
  friend std::ostream& operator<<(std::ostream& os, AIEndPoint const& end_point);
};
