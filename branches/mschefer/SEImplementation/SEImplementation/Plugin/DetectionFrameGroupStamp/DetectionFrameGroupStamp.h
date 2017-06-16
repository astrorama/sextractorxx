/*
 * DetectionFrameGroupStamp.h
 *
 *  Created on: May 5, 2017
 *      Author: mschefer
 */

#ifndef _SEIMPLEMENTATION_PLUGIN_DETECTIONFRAMEGROUPSTAMP_DETECTIONFRAMEGROUPSTAMP_H_
#define _SEIMPLEMENTATION_PLUGIN_DETECTIONFRAMEGROUPSTAMP_DETECTIONFRAMEGROUPSTAMP_H_


#include "SEFramework/Property/Property.h"
#include "SEFramework/Image/Image.h"

namespace SExtractor {

class DetectionFrameGroupStamp : public Property {

public:

  virtual ~DetectionFrameGroupStamp() = default;

  DetectionFrameGroupStamp(std::shared_ptr<DetectionImage> stamp, PixelCoordinate top_left,
      std::shared_ptr<WeightImage> weight_stamp) : m_stamp(stamp), m_weight_stamp(weight_stamp), m_top_left(top_left) {}

  // Returns the stamp image
  const DetectionImage& getStamp() const {
    return *m_stamp;
  }

  // Returns the stamp's associated weight image
  const DetectionImage& getWeightStamp() const {
    return *m_weight_stamp;
  }

  PixelCoordinate getTopLeft() const {
    return m_top_left;
  }

private:
  std::shared_ptr<DetectionImage> m_stamp;
  std::shared_ptr<WeightImage> m_weight_stamp;
  PixelCoordinate m_top_left;

};


} /* namespace SExtractor */




#endif /* _SEIMPLEMENTATION_PLUGIN_DETECTIONFRAMEGROUPSTAMP_DETECTIONFRAMEGROUPSTAMP_H_ */
