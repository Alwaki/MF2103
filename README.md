# MF2103
Course project. Consists of tasks with STM32 ARM Cortex M4 board, motor control board as well as motor with encoder.

Task 1: PID control.
---------------------
Finished. Note that build files are not included for brevity (100+ files).
PI control with anti-windup. Parameters are not optimally tuned, merely tuned until functional.

Task 2: RTOS.
---------------------
Finished. File for basic case (osDelay implementation) is also included. Bonus implementation (osSignalWait & Timers) is included as application2.c.

Task 3: Distributed systems using TCP/IP.
---------------------
Finished. Client application file uses 4 threads (including main with bootup sequence) for sampling, actuation and communication. Server uses 3 threads (including main with bootup sequence), for reference, calculation, and communication. TCP handshake is initiated at start, and when connection is interrupted (power loss, ethernet cable unplugged). Bonus implementation is done, motor is signalled to stop within one sampling time from disconnect.
