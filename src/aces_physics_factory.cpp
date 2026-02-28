#include "aces/aces_physics_factory.hpp"

#include <iostream>

#include "aces/physics/aces_dms.hpp"
#include "aces/physics/aces_dms_fortran.hpp"
#include "aces/physics/aces_fortran_bridge.hpp"
#include "aces/physics/aces_lightning.hpp"
#include "aces/physics/aces_lightning_fortran.hpp"
#include "aces/physics/aces_megan.hpp"
#include "aces/physics/aces_megan_fortran.hpp"
#include "aces/physics/aces_native_example.hpp"
#include "aces/physics/aces_sea_salt.hpp"
#include "aces/physics/aces_sea_salt_fortran.hpp"
#include "aces/physics/aces_soil_nox.hpp"
#include "aces/physics/aces_soil_nox_fortran.hpp"

/**
 * @file aces_physics_factory.cpp
 * @brief Implementation of the PhysicsFactory for scheme instantiation.
 */

namespace aces {

/**
 * @brief Creates a physics scheme based on the provided configuration.
 *
 * Supports both native C++ schemes and Fortran-based schemes via a bridge.
 *
 * @param config Configuration for the physics scheme.
 * @return A unique pointer to the created PhysicsScheme, or nullptr if type is
 * unknown.
 */
std::unique_ptr<PhysicsScheme> PhysicsFactory::CreateScheme(
    const PhysicsSchemeConfig& config) {
  std::unique_ptr<PhysicsScheme> scheme;

  if (config.name == "sea_salt") {
    scheme = std::make_unique<SeaSaltScheme>();
  } else if (config.name == "sea_salt_fortran") {
#ifdef ACES_HAS_FORTRAN
    scheme = std::make_unique<SeaSaltFortranScheme>();
#endif
  } else if (config.name == "megan") {
    scheme = std::make_unique<MeganScheme>();
  } else if (config.name == "megan_fortran") {
#ifdef ACES_HAS_FORTRAN
    scheme = std::make_unique<MeganFortranScheme>();
#endif
  } else if (config.name == "dms") {
    scheme = std::make_unique<DMSScheme>();
  } else if (config.name == "dms_fortran") {
#ifdef ACES_HAS_FORTRAN
    scheme = std::make_unique<DMSFortranScheme>();
#endif
  } else if (config.name == "lightning") {
    scheme = std::make_unique<LightningScheme>();
  } else if (config.name == "lightning_fortran") {
#ifdef ACES_HAS_FORTRAN
    scheme = std::make_unique<LightningFortranScheme>();
#endif
  } else if (config.name == "soil_nox") {
    scheme = std::make_unique<SoilNoxScheme>();
  } else if (config.name == "soil_nox_fortran") {
#ifdef ACES_HAS_FORTRAN
    scheme = std::make_unique<SoilNoxFortranScheme>();
#endif
  } else if (config.language == "fortran" ||
             config.name == "fortran_bridge_example") {
#ifdef ACES_HAS_FORTRAN
    std::cout << "ACES_PhysicsFactory: Creating Fortran scheme " << config.name
              << std::endl;
    scheme = std::make_unique<FortranBridgeExample>();
#else
    std::cerr << "ACES_PhysicsFactory: Error - Fortran scheme " << config.name
              << " requested but Fortran support is disabled." << std::endl;
#endif
  } else {
    // Default to Native C++
    std::cout << "ACES_PhysicsFactory: Creating Native scheme " << config.name
              << std::endl;
    scheme = std::make_unique<NativePhysicsExample>();
  }

  return scheme;
}

}  // namespace aces
