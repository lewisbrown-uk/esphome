#pragma once

#include "esphome/components/output/float_output.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace mcp4661 {

enum MemoryAddress {
  VOLATILE_WIPER_0 = 0x00,
  VOLATILE_WIPER_1 = 0x01,
  NON_VOLATILE_WIPER_0 = 0x02,
  NON_VOLATILE_WIPER_1 = 0x03,
  VOLATILE_TCON_REG = 0X04,
  STATUS_REG = 0X05,
};

enum Command {
  READ = 0x00,
  INCREMENT = 0x01,
  DECREMENT = 0x02,
  WRITE = 0x03,
};

class MCP4661Output;

class MCP4661Channel : public Component, public output::FloatOutput, public Parented<MCP4661Output> {
 public:
  void set_channel(uint8_t wiper) { wiper_ = wiper; update_wiper_address(); }
  void set_volatility(bool is_volatile) { is_volatile_ = is_volatile; update_wiper_address(); }

 protected:
  friend class MCP4661Output;

  void write_state(float state) override;
  void update_wiper_address(void);

  uint8_t wiper_;
  bool is_volatile_;
  MemoryAddress wiper_address_;
  uint16_t wiper_value_max_;
  float wiper_step_size_;
};

class MCP4661Output : public Component, public i2c::I2CDevice {
 public:
  MCP4661Output() {}

  void register_channel(MCP4661Channel * channel);

  void set_number_of_bits(int bits) { number_of_bits_ = bits; }
  void set_number_of_wipers(int wipers) { number_of_wipers_ = wipers; }

  void setup() override;
  void dump_config() override;

 protected:
  friend MCP4661Channel;

  enum ErrorCode { NONE = 0, COMMUNICATION_FAILED } error_code_{NONE};
  
  void set_wiper_value(MemoryAddress wiper_address, uint16_t value);
  static uint8_t construct_command_byte(MemoryAddress memory_address, Command command, uint16_t data);

  int number_of_bits_, number_of_wipers_;
  float wiper_step_size_;
  uint16_t wiper_value_max_;
  uint8_t min_channel_{0xFF};
  uint8_t max_channel_{0x00};

};

}  // namespace mcp4661
}  // namespace esphome
