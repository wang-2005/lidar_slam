/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file EqVIOSymmetry.h
 * @brief EqVIO symmetry actions and lift helpers.
 * @author Rohan Bansal
 */

#pragma once

#include <gtsam/base/GroupAction.h>
#include <gtsam/base/Matrix.h>
#include <gtsam/base/Vector.h>
#include <gtsam_unstable/navigation/EqVIOCommon.h>
#include <gtsam_unstable/navigation/EqVIOState.h>
#include <gtsam_unstable/dllexport.h>

namespace gtsam {
namespace eqvio {

/// Right group action on the sensor-only block.
GTSAM_UNSTABLE_EXPORT SensorState sensorStateGroupAction(
    const VioGroup& X, const SensorState& sensor);
/// Right group action on full state.
GTSAM_UNSTABLE_EXPORT State stateGroupAction(const VioGroup& X,
                                             const State& state);
/// Right group action on vision measurements.
GTSAM_UNSTABLE_EXPORT VisionMeasurement outputGroupAction(
    const VioGroup& X, const VisionMeasurement& measurement,
    const std::shared_ptr<const CameraModel>& camera);

/// Continuous-time lift map from IMU velocity to VioGroup tangent.
GTSAM_UNSTABLE_EXPORT Vector liftVelocity(const State& state,
                                          const IMUInput& velocity);
/// Discrete-time lift map from IMU velocity to VioGroup increment.
GTSAM_UNSTABLE_EXPORT VioGroup liftVelocityDiscrete(const State& state,
                                                    const IMUInput& velocity,
                                                    double dt);

/// Integrate system dynamics forward by dt.
GTSAM_UNSTABLE_EXPORT State integrateSystemFunction(
    const State& state, const IMUInput& velocity, double dt);
/// Generate ideal camera measurements from state.
GTSAM_UNSTABLE_EXPORT VisionMeasurement measureSystemState(
    const State& state, const std::shared_ptr<const CameraModel>& camera);

/// Right action phi(xi, X) = stateGroupAction(X, xi).
struct GTSAM_UNSTABLE_EXPORT VIOSymmetry
    : public GroupAction<VIOSymmetry, VioGroup, State> {
  static constexpr ActionType type = ActionType::Right;

  /// Evaluate right action phi(xi, X).
  State operator()(const State& xi, const VioGroup& X,
                   OptionalJacobian<Eigen::Dynamic, Eigen::Dynamic> H_xi = {},
                   OptionalJacobian<Eigen::Dynamic, Eigen::Dynamic> H_X = {})
      const;
};

}  // namespace eqvio
}  // namespace gtsam
