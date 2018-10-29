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

char const* ConnectToEndPoint::state_str_impl(state_type run_state) const
{
  switch(run_state)
  {
    AI_CASE_RETURN(ConnectToEndPoint_start);
    AI_CASE_RETURN(ConnectToEndPoint_ready);
    AI_CASE_RETURN(ConnectToEndPoint_done);
  }
  ASSERT(false);
  return "UNKNOWN STATE";
}

void ConnectToEndPoint::multiplex_impl(state_type run_state)
{
  switch (run_state)
  {
    case ConnectToEndPoint_start:
      // Advance to state ConnectToEndPoint_ready when m_end_point is usable.
      set_state(ConnectToEndPoint_ready);
      wait(1);
      m_end_point.run(m_get_addr_info, this, 1 COMMA_DEBUG_ONLY(mSMDebug));     // Do hostname lookup, if necessary.
      break;
    case ConnectToEndPoint_ready:
      /* FALL-THROUGH */
    case ConnectToEndPoint_done:
      finish();
      break;
  }
}

} // namespace task
