/**
 * @file LeastSquareEngineManager.h
 * @date April 16, 2019
 * @author Alejandro Alvarez Ayllon
 */

#ifndef MODELFITTING_LEASTSQUAREENGINEMANAGER_H
#define MODELFITTING_LEASTSQUAREENGINEMANAGER_H

#include "ModelFitting/Engine/LeastSquareEngine.h"

namespace ModelFitting {

/**
 * @class LeastSquareEngineManager
 *
 * @brief
 * Keep a registry of supported engines, and bridge their factory methods
 *
 * @details
 * Engines can register themselves at compilation time using StaticEngine. In this manner,
 * the final binary can have an arbitrary number of supported engines depending on the available
 * dependencies when compiling.
 *
 * Different engines may have different parametrization, so it is up to the respective
 * factory methods to initialize them. The only parameter passed down to the factories is
 * the maximum number of iterations.
 */
class LeastSquareEngineManager {
public:

  /**
   * Factory methods must return a shared pointer to LeastSquareEngine, and receive an unsigned
   * that sets the maximum number of iterations.
   */
  using FactoryMethod = std::function<std::shared_ptr<LeastSquareEngine>(unsigned)>;

  /**
   * Register a new engine.
   * @param name
   *    The name of the engine. Case insensitive.
   * @param factory_method
   *    The function to call for creating a new instance.
   */
  static void registerEngine(const std::string& name, FactoryMethod factory_method);

  /**
   * @return A list of known engines.
   */
  static std::vector<std::string> getImplementations();

  /**
   * Create an instance of an engine 'name'
   * @param name
   *    The name of the engine, as passed to registerEngine. Case insensitive.
   * @param max_iterations
   *    Maximum number of iterations.
   * @return
   *    A new instance of the specific engine. An Elements::Exception is thrown if the engine name
   *    is unknown.
   */
  static std::shared_ptr<LeastSquareEngine> create(const std::string &name, unsigned max_iterations = 1000);

  /**
   * Helper struct to register statically concrete implementations.
   */
  struct StaticEngine {
    StaticEngine(const std::string& name, LeastSquareEngineManager::FactoryMethod factory_method) {
      LeastSquareEngineManager::registerEngine(name, factory_method);
    }
  };
};

}  // end of namespace ModelFitting

#endif /* MODELFITTING_LEASTSQUAREENGINEMANAGER_H */