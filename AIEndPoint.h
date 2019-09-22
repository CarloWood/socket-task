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

  bool is_ready() const { return !m_cached || m_cached->is_ready(); }

  // Reset the union iterator.
  bool reset();

  // Return the current address; only call this when the last call to reset() or next() returned true.
  evio::SocketAddress current() const;

  // Advance to the next address, if any.
  bool next();

  // Return the to be resolved (possibly already resolved) hostname.
  std::string hostname() const { return m_cached->hostname(); }

  // Create (if still empty) and run a GetAddrInfo task, calling cont() on the parent when ready.
  void run(boost::intrusive_ptr<task::GetAddrInfo>& task, AIStatefulTask* parent, AIStatefulTask::condition_type conditions COMMA_CWDEBUG_ONLY(bool debug_resolver));

  // Support writing to an ostream.
  friend std::ostream& operator<<(std::ostream& os, AIEndPoint const& end_point);
};
