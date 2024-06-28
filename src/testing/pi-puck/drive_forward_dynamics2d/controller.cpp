/**
 *
 * @author Michael Allwright <mallwright@learnrobotics.io>
 */

#include <argos3/testing/pi-puck/drive_forward_dynamics2d/controller.h>

#include <argos3/plugins/robots/pi-puck/control_interface/ci_pipuck_differential_drive_actuator.h>

namespace argos {

   /****************************************/
   /****************************************/

   void CTestController::Init(TConfigurationNode& t_tree) {
      CCI_PiPuckDifferentialDriveActuator* pcActuator =
         GetActuator<CCI_PiPuckDifferentialDriveActuator>("pipuck_differential_drive");
      pcActuator->SetLinearVelocity(0.1, 0.1); // 10 centimeter per second forwards
   }

   /****************************************/
   /****************************************/

   REGISTER_CONTROLLER(CTestController, "test_controller");

}



