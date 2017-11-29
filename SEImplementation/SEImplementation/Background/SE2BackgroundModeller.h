/*
 * Created on Jan 05, 2015
 * @author: mkuemmel@usm.lmu.de
 *
 * Date:     $Date$
 * Revision: $Revision$
 * Author:   $Author$
 */
#ifndef BACKGROUNDMODELLER_H
#define	BACKGROUNDMODELLER_H

#include <vector>
#include <memory>
#include <boost/filesystem.hpp>
#include "fitsio.h"
#include "SEFramework/Image/Image.h"
#include "SEImplementation/Background/BackgroundDefine.h"
#include "SEImplementation/Background/TypedSplineModelWrapper.h"

namespace SExtractor {

class SE2BackgroundModeller {

public:
  SE2BackgroundModeller(std::shared_ptr<DetectionImage> image, std::shared_ptr<WeightImage> variance_map=nullptr, std::shared_ptr<Image<unsigned char>> mask=nullptr, const unsigned char mask_type_flag=0x0001);
  SE2BackgroundModeller(const boost::filesystem::path& fits_filename, const boost::filesystem::path& weight_filename,const boost::filesystem::path& mask_filename,  const int mask_type, const int weight_type_flag=0x0001);
  virtual ~SE2BackgroundModeller();

  void createModels(std::shared_ptr<TypedSplineModelWrapper<SeFloat>> &bckPtr, std::shared_ptr<TypedSplineModelWrapper<SeFloat>> &sigPtr, PIXTYPE &sigFac, const size_t *bckCellSize, const PIXTYPE varianceThreshold,  const size_t *filterBoxSize, const float &filterThreshold=0.0);
  void createSE2Models(std::shared_ptr<TypedSplineModelWrapper<SeFloat>> &bckPtr, std::shared_ptr<TypedSplineModelWrapper<SeFloat>> &sigPtr, PIXTYPE &sigFac, const size_t *bckCellSize,  const WeightImage::PixelType varianceThreshold,  const size_t *filterBoxSize, const float &filterThreshold=0.0);

  //
  PIXTYPE* getWhtMeanVals();
  void computeScalingFactor(PIXTYPE* whtMeanVals, PIXTYPE* bckSigVals, PIXTYPE& sigFac, const size_t nGridPoints);
  ///
private:
  void getMinIncr(size_t &nElements, long *incr, size_t *subImgNaxes);
  bool checkCompatibility(const boost::filesystem::path itsInputWeightName, fitsfile* itsInputWeight, const size_t* itsNaxes);
  bool checkCompatibility(fitsfile* itsInputWeight, const size_t* itsNaxes);

  void filter(PIXTYPE* bckVals, PIXTYPE* sigmaVals, const size_t* gridSize, const size_t* filterSize, const float &filterThreshold=0.0);
  void replaceUNDEF(PIXTYPE* bckVals, PIXTYPE* sigmaVals,const size_t* gridSize);
  void filterMedian(PIXTYPE* bckVals, PIXTYPE* sigmaVals,const size_t* gridSize, const size_t* filterSize, const float filterThresh=0.0);

  void rescaleThreshold(PIXTYPE &weightVarThreshold, const PIXTYPE &weightThreshold);

  // input to create the object
  boost::filesystem::path itsInputMaskName;
  boost::filesystem::path itsInputFileName;
  boost::filesystem::path itsInputWeightName;
  int itsWeightTypeFlag;
  unsigned char itsMaskType;

  std::shared_ptr<DetectionImage> itsImage=nullptr;
  std::shared_ptr< WeightImage> itsVariance=nullptr;
  std::shared_ptr< Image<unsigned char>> itsMask=nullptr;

  fitsfile* itsInputMask=NULL;
  fitsfile* itsInputFits=NULL;
  fitsfile* itsInputWeight=NULL;
  bool itsHasVariance=false;
  bool itsHasMask=false;
  //
  PIXTYPE* itsWhtMeanVals=NULL;
  //
  // data from the input image
  int itsBitpix=0;
  int itsNaxis=0;
  size_t itsNaxes[2]={0,0};
};

} // end of namespace SExtractor

#endif	// BACKGROUNDMODELLER_H

