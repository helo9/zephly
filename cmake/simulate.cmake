# SPDX-License-Identifier: Apache-2.0

add_custom_target(simulate
  COMMAND
  echo "Simulation with jMAVSim was started."
  DEPENDS 
  jMAVSim_run
  WORKING_DIRECTORY ${APPLICATION_BINARY_DIR}
  USES_TERMINAL
  )
