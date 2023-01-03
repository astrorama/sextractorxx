CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

#===============================================================================
# Load elements_subdir macro here
# Examples:
#   For declaring a project module:
#         elements_subdir(ElementsExamples)
#===============================================================================
elements_subdir(SEPythonModule)

#===============================================================================
# Load elements_depends_on_subdirs macro here
#   For creating a dependency onto an other accessible module
#         elements_depends_on_subdirs(ElementsKernel)
#===============================================================================
elements_depends_on_subdirs(ElementsKernel)
elements_depends_on_subdirs(SEImplementation)
elements_depends_on_subdirs(SEMain)

#===============================================================================
# Add the find_package macro (a pure CMake command) here to locate the
# libraries.
# Examples:
#          find_package(CppUnit)
#===============================================================================
find_package(PythonInterp ${PYTHON_EXPLICIT_VERSION} COMPONENTS Development)
set(PYTHON_SUFFIX "${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
find_package(Boost 1.63 COMPONENTS "numpy${PYTHON_SUFFIX}" "python${PYTHON_SUFFIX}")

if (Boost_FOUND)
  #===============================================================================
  # Declare the library dependencies here
  # Example:
  #         elements_add_library(ElementsExamples src/Lib/*.cpp
  #                     INCLUDE_DIRS Boost ElementsKernel
  #                     LINK_LIBRARIES Boost ElementsKernel
  #                     PUBLIC_HEADERS ElementsExamples)
  #===============================================================================
  elements_add_python_module(SEPythonModule
          src/lib/*.cpp
          LINK_LIBRARIES SEImplementation SEMain Boost PythonInterp
          )

  #===============================================================================
  # Declare the executables here
  # Example:
  # elements_add_executable(ElementsProgramExample src/Program/ProgramExample.cpp
  #                        INCLUDE_DIRS Boost ElementsExamples
  #                        LINK_LIBRARIES Boost ElementsExamples)
  #===============================================================================

  #===============================================================================
  # Declare the Boost tests here
  # Example:
  # elements_add_unit_test(BoostClassExample tests/src/Boost/ClassExample_test.cpp
  #                       EXECUTABLE BoostClassExample_test
  #                       INCLUDE_DIRS ElementsExamples
  #                       LINK_LIBRARIES ElementsExamples TYPE Boost)
  #===============================================================================

  #===============================================================================
  # Use the following macro for python modules, scripts and aux files:
  #  elements_install_python_modules()
  #  elements_install_scripts()
  #===============================================================================

  #===============================================================================
  # Declare the Python programs here
  # Examples :
  # elements_add_python_program(PythonProgramExample
  #                             ElementsExamples.PythonProgramExample)
  #===============================================================================

  #===============================================================================
  # Add the elements_install_conf_files macro
  # Examples:
  #          elements_install_conf_files()
  #===============================================================================
endif (Boost_FOUND)
