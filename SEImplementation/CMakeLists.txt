CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

#===============================================================================
# Load elements_subdir macro here
# Examples:
#   For declaring a project module:
#         elements_subdir(ElementsExamples)
#===============================================================================
elements_subdir(SEImplementation)

#===============================================================================
# Load elements_depends_on_subdirs macro here
#   For creating a dependency onto an other accessible module
#         elements_depends_on_subdirs(ElementsKernel)
#===============================================================================
elements_depends_on_subdirs(ElementsKernel)
elements_depends_on_subdirs(AlexandriaKernel) # From Alexandria
elements_depends_on_subdirs(Configuration) # From Alexandria
elements_depends_on_subdirs(Table) # From Alexandria
elements_depends_on_subdirs(NdArray) # From Alexandria
elements_depends_on_subdirs(SEUtils)
elements_depends_on_subdirs(SEFramework)
elements_depends_on_subdirs(ModelFitting)
elements_depends_on_subdirs(SEUtils)
elements_depends_on_subdirs(Pyston)

#===============================================================================
# Add the find_package macro (a pure CMake command) here to locate the
# libraries.
# Examples:
#          find_package(CppUnit)
#===============================================================================

find_package(PythonInterp ${PYTHON_EXPLICIT_VERSION} REQUIRED)
find_package(PythonLibs ${PYTHON_EXPLICIT_VERSION} REQUIRED)
find_package(BoostPython ${PYTHON_EXPLICIT_VERSION})
find_package(Boost 1.53 REQUIRED)
find_package(OnnxRuntime)
find_package(Log4CPP REQUIRED)

if (${Boost_VERSION} LESS "106700")
    message(WARNING "
Numerical differentiation is available starting from Boost 106700 (${Boost_VERSION} found instead).
The model fitting plugin will use a potentially less robust fallback method for computing error margins!")
endif ()

if (${PYTHON_VERSION_MAJOR} VERSION_LESS 3)
    message(DEPRECATION "Python 2 support is deprecated and will be removed (found ${PYTHON_VERSION_STRING})")
endif ()

#===============================================================================
# Declare the library dependencies here
# Example:
#         elements_add_library(ElementsExamples src/Lib/*.cpp
#                     LINK_LIBRARIES Boost ElementsKernel
#                     INCLUDE_DIRS Boost ElementsKernel
#                     PUBLIC_HEADERS ElementsExamples)
#===============================================================================

set(OPT_INCLUDES)
set(OPT_LIBRARIES)

file(GLOB SE_PYTHON_SRC src/lib/PythonConfig/*.cpp)
list(REMOVE_ITEM SE_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/PythonConfig/PythonModule.cpp)

file(GLOB PLUGIN_SRC src/lib/Plugin/*/*.cpp)
file(GLOB SEGMENTATION_SRC src/lib/Segmentation/*.cpp)
file(GLOB COMMON_SRC src/lib/Common/*.cpp)

if(NOT OnnxRuntime_FOUND)
  message("ONNX Runtime not found")
  file(GLOB ONNXPLUGIN_SRC src/lib/Plugin/Onn*/*.cpp)
  file(GLOB ML_SEGMENTATION_SRC src/lib/Segmentation/ML*.cpp)
  file(GLOB ONNX_COMMON_SRC src/lib/Common/Onnx*.cpp)
  list(REMOVE_ITEM PLUGIN_SRC ${ONNXPLUGIN_SRC})
  list(REMOVE_ITEM SEGMENTATION_SRC ${ML_SEGMENTATION_SRC})
  list(REMOVE_ITEM COMMON_SRC ${ONNX_COMMON_SRC})
else()
  list(APPEND OPT_INCLUDES ${OnnxRuntime_INCLUDE_DIRS})
  list(APPEND OPT_LIBRARIES OnnxRuntime)
  add_definitions(-DWITH_ML_SEGMENTATION)
  add_definitions(-DWITH_ONNX_MODELS)
endif()

elements_add_library(SEImplementation
            src/lib/Background/*.cpp
            src/lib/Background/*/*.cpp
            src/lib/Partition/*.cpp
            ${SEGMENTATION_SRC}
            src/lib/Grouping/*.cpp
            src/lib/Configuration/*.cpp
            src/lib/Output/*.cpp
            src/lib/Measurement/*.cpp
            src/lib/CheckImages/*.cpp
            src/lib/Deblending/*.cpp
            src/lib/Image/*.cpp
            ${COMMON_SRC}
            src/lib/Prefetcher/*.cpp
            src/lib/Property/*.cpp
            ${PLUGIN_SRC}
            ${SE_PYTHON_SRC}
            LINK_LIBRARIES
              ElementsKernel Configuration Table
              SEUtils SEFramework ModelFitting
              BoostPython PythonLibs PythonInterp
              Pyston
              ${OPT_LIBRARIES}
            INCLUDE_DIRS ${BoostPython_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS} ${OPT_INCLUDES}
            PUBLIC_HEADERS SEImplementation)

#===============================================================================
# SEImplementation has a part that needs to be reachable from Python.
#===============================================================================
elements_add_python_module(SEPythonConfig
		src/lib/PythonConfig/PythonModule.cpp
		LINK_LIBRARIES SEImplementation
		INCLUDE_DIRS ${Boost_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS}
)

#===============================================================================
# Declare the executables here
# Example:
# elements_add_executable(ElementsProgramExample src/Program/ProgramExample.cpp
#                        LINK_LIBRARIES Boost ElementsExamples
#                        INCLUDE_DIRS Boost ElementsExamples)
#===============================================================================

#===============================================================================
# Declare the Boost tests here
# Example:
# elements_add_unit_test(BoostClassExample tests/src/Boost/ClassExample_test.cpp
#                       EXECUTABLE BoostClassExample_test
#                       INCLUDE_DIRS ElementsExamples
#                       LINK_LIBRARIES ElementsExamples TYPE Boost)
#===============================================================================
elements_add_unit_test(KappaSigmaBinning_test tests/src/Background/KappaSigmaBinning_test.cpp
		             LINK_LIBRARIES SEImplementation
		             TYPE Boost)
elements_add_unit_test(MedianFilterImage_test tests/src/Background/MedianFilterImage_test.cpp
		             LINK_LIBRARIES SEImplementation
		             TYPE Boost)
elements_add_unit_test(ReplaceUndefImage_test tests/src/Background/ReplaceUndefImage_test.cpp
		             LINK_LIBRARIES SEImplementation
		             TYPE Boost)
elements_add_unit_test(PixelCentroid_test tests/src/Plugin/PixelCentroid/PixelCentroid_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(DetectionFramePixelValues_test tests/src/Plugin/DetectionFramePixelValues/DetectionFramePixelValues_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(PixelBoundaries_test tests/src/Plugin/PixelBoundaries/PixelBoundaries_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(DetectionFrameSourceStamp_test tests/src/Plugin/DetectionFrameSourceStamp/DetectionFrameSourceStamp_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(Lutz_test tests/src/Segmentation/LutzSegmentation_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(MinAreaPartitionStep_test tests/src/Partition/MinAreaPartitionStep_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(AttractorsPartitionStep_test tests/src/Partition/AttractorsPartitionStep_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(MultiThresholdPartitionStep_test tests/src/Partition/MultiThresholdPartitionStep_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(OverlappingBoundariesCriteria_test tests/src/Grouping/OverlappingBoundariesCriteria_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(ExternalFlag_test tests/src/Plugin/ExternalFlag/ExternalFlag_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(FluxRadius_test tests/src/Plugin/FluxRadius/FluxRadius_test.cpp
		             LINK_LIBRARIES SEImplementation
		             TYPE Boost)
elements_add_unit_test(GrowthCurve_test tests/src/Plugin/GrowthCurve/GrowthCurve_test.cpp
		             LINK_LIBRARIES SEImplementation
		             TYPE Boost)
elements_add_unit_test(AperturePhotometry_test tests/src/Plugin/AperturePhotometry/AperturePhotometry_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(IsophotalFlux_test tests/src/Plugin/IsophotalFlux/IsophotalFlux_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(JacobianGroup_test tests/src/Plugin/Jacobian/JacobianGroupTask_test.cpp
		             LINK_LIBRARIES SEImplementation
		             TYPE Boost)
elements_add_unit_test(JacobianSource_test tests/src/Plugin/Jacobian/JacobianSourceTask_test.cpp
		             LINK_LIBRARIES SEImplementation
		             TYPE Boost)
if (NOT WITHOUT_MODELFITTING)
elements_add_unit_test(MoffatModelFitting_test tests/src/Plugin/MoffatModelFitting/MoffatModelFitting_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
endif()
elements_add_unit_test(PsfTask_test tests/src/Plugin/Psf/PsfTask_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(ImagePsf_test tests/src/Image/ImagePsf_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(ImageInterfaceTraits_test tests/src/Image/ImageInterfaceTraits_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(BackgroundConvolution_test tests/src/Segmentation/BackgroundConvolution_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)
elements_add_unit_test(AssocMode_test tests/src/Plugin/AssocMode/AssocMode_test.cpp
                     LINK_LIBRARIES SEImplementation
                     TYPE Boost)

#===============================================================================
# Declare the Python programs here
# Examples :
# elements_add_python_program(PythonProgramExample
#                             ElementsExamples.PythonProgramExample)
#===============================================================================

#===============================================================================
# Use the following macro for python modules, scripts and aux files:
#  elements_install_python_modules()
#  elements_install_scripts()
#===============================================================================

#===============================================================================
# Add the elements_install_conf_files macro
# Examples:
#          elements_install_conf_files()
#===============================================================================
