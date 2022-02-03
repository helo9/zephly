.. _getting_started:

Getting Started
===============

This guide will help you setting up your development environment and enables
you developing frickelflightcontrol.

#. Install `Zephyr RTOS`_ following the `Zephyr Getting Started`_ guide and ensure you can build the example.

#. Use west to clone this project

   .. code-block:: bash

       west init -m git@github.com:helo9/zephly zephly-project
   
   Change into the ``zephly-project`` folder and run

   .. code-block:: bash

      west update

   This will clone relevant repositories intro ffc-project. The content of zephly can be found in ``zephly-project/zephly``.

#. Build the pwm sample for ``openpilot_cc3d``

   .. code-block:: bash

      west build --board=openpilot_cc3d samples/basic/pwm_out

.. _Zephyr RTOS: https://zephyrproject.org
.. _Zephyr Getting Started: https://docs.zephyrproject.org/latest/getting_started/index.html
