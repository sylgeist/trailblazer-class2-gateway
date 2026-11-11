// Build-time configuration for the Class 2 gateway (Rugged MEGA-ET ST, ATmega2560).
// Pin and timer choices are explained in docs/pins-and-timers.md.
#pragma once

// ---- Safety switches -------------------------------------------------------
// Everything that drives the vehicle stays OFF until it has been validated on
// the bench. With all three at 0 the gateway only listens and logs.
#define ENABLE_CLASS2_TX   0   // transmit frames from the message table
#define ENABLE_TORQUE_PWM  0   // 128 Hz delivered/requested torque PWM to the EBCM
#define ENABLE_SPEED_OUT   0   // synthesized rear-speed square wave to the EBCM

// ---- Pins ------------------------------------------------------------------
#define PIN_CAN_INT           2    // INT4, MCP2515 interrupt (active low)
#define PIN_VPW_RX            3    // INT5, Class 2 receive (discrete interface)
#define PIN_VPW_TX            4    // Class 2 transmit drive (discrete interface)
#define PIN_SPEED_OUT         5    // OC3A, rear-speed square wave
#define PIN_TORQUE_DELIVERED  11   // OC1A, 128 Hz PWM
#define PIN_TORQUE_REQUESTED  12   // OC1B, 128 Hz PWM
#define PIN_STATUS_LED        13
#define PIN_CAN_CS            53   // hardware SS, MCP2515 chip select

// ---- CAN (MaxxECU default protocol) ----------------------------------------
#define CAN_SPEED        CAN_500KBPS
#define CAN_CRYSTAL      MCP_16MHZ   // VERIFY against the crystal on the FeatherWing
#define CAN_TIMEOUT_MS   500         // engine data older than this is treated as stale

// ---- Class 2 ---------------------------------------------------------------
// The gateway stands in for the factory PCM, which sits at node $10.
#define GATEWAY_NODE_ADDR  0x10

// ---- Rear speed output -----------------------------------------------------
// UNCONFIRMED: measure pulses per mile on the stock PCM->EBCM speed wire.
#define SPEED_PULSES_PER_MILE  4000UL
