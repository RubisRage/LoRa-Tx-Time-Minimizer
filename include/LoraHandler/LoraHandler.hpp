#pragma once

#include <DutyCycleManager/DutyCycleManager.hpp>
#include <cstdint>
#include <types/LoraTypes.hpp>
#include <types/Message.hpp>

#define INITIAL_INTERVAL_BETWEEN_TX std::chrono::milliseconds(10000)

class LoraHandler {
public:
  LoraHandler();
  LoraHandler(LoraHandler &&) = delete;
  LoraHandler(const LoraHandler &) = delete;
  LoraHandler &operator=(LoraHandler &&) = delete;
  LoraHandler &operator=(const LoraHandler &) = delete;
  ~LoraHandler() = default;

  /**
   * Initialization function. Must be called at least once before
   * using any other function.
   */
  void setup(const LoRaConfig &config, void (*onReceive)(int) = nullptr);

  /**
   * Send message using LoRa library.
   *
   * @return true if message sent, false if duty cycle restriction would
   * be violated by transmission.
   */
  bool sendMessage(Message);

  /**
   * Update LoRa configuration parameters.
   */
  void updateConfig(const LoRaConfig &config);

  /*
   * Must be called periodically for transmission state to be reflected
   * correctly.
   */
  void updateTransmissionState();

  /**
   * @return true if transmission is possible, false otherwise. A transmission
   * is possible when a transmission is not currently happening and a duty
   * cycle of 1% would be respected if a transmission were to start at the
   * current moment
   */
  bool canTransmit();

  /**
   * Get last received message.
   *
   * @return true if returned message is valid, false otherwise.
   */
  bool get(Message &);

private:
  Message lastReceived;
  bool validMessage;
  DutyCycleManager dutyCycleManager;
  volatile bool transmitting;
  volatile bool txDone;

  void onReceive(int packetSize);
};

extern LoraHandler loraHandler;
