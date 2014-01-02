#include "crv.h"

namespace crv
{

const std::string Tracer::s_value_prefix = "v!";
const std::string Encoder::s_time_prefix = "t!";
const std::string Encoder::s_rf_prefix = "rf!";
const std::string Encoder::s_pf_prefix = "pf!";
const std::string Encoder::s_ldf_prefix = "ldf!";

Tracer& tracer() {
  static Tracer s_tracer;
  return s_tracer;
}

bool Tracer::append_guard(const Internal<bool>& internal)
{
  bool direction = true;
  if (m_flip_iter == m_flips.cend())
  {
    m_flips.push_back(Flip());
    assert(m_flips.back().direction == direction);
  }
  else
  {
    direction = m_flip_iter->direction;
    m_flip_iter++;
  }

  if (direction)
    m_guard = m_guard and internal.term;
  else
    m_guard = m_guard and !internal.term;

  return direction;
}

}
