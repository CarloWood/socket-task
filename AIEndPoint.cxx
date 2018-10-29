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

void AIEndPoint::run(boost::intrusive_ptr<task::GetAddrInfo>& task, AIStatefulTask* parent, AIStatefulTask::condition_type condition COMMA_DEBUG_ONLY(bool debug_resolver))
{
  // Only call run() once, and pass it a default constructed boost::intrusive_ptr<task::GetAddrInfo>.
  ASSERT(!task);
  // Did we ask for a hostname lookup and did it not finish yet?
  if (m_cached && !m_cached->is_ready())
  {
    task = new task::GetAddrInfo(DEBUG_ONLY(debug_resolver));
    task->init(m_cached);
    task->run(resolver::Resolver::instance().get_handler(), [parent, condition](bool){ parent->signal(condition); });
    return;
  }
  // No need to run at all.
  parent->signal(condition);
}
