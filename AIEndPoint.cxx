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
