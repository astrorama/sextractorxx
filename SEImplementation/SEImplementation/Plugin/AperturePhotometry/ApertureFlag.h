/*
 * ApertureFlag.h
 *
 *  Created on: Oct 09, 2018
 *      Author: Alejandro Alvarez Ayllon
 */

#ifndef _SEIMPLEMENTATION_PLUGIN_APERTUREPHOTOMETRY_APERTUREFLAG_H_
#define _SEIMPLEMENTATION_PLUGIN_APERTUREPHOTOMETRY_APERTUREFLAG_H_

#include "SEUtils/Types.h"
#include "SEFramework/Property/Property.h"
#include "SEFramework/Source/SourceFlags.h"
#include <vector>

namespace SExtractor {

/**
 * @class ApertureFlag
 * @brief Aperture photometry flag
 */
class ApertureFlag : public Property {
public:

  virtual ~ApertureFlag() = default;

  ApertureFlag(const std::vector<Flags>& flags): m_flags{flags} {}

  const std::vector<Flags>& getFlags() const {
    return m_flags;
  }

private:
  std::vector<Flags> m_flags;
};

} /* namespace SExtractor */

#endif /* _SEIMPLEMENTATION_PLUGIN_APERTUREPHOTOMETRY_APERTUREFLAG_H_ */