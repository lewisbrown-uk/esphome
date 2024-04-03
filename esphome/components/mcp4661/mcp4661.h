#pragma once

#include "esphome/components/output/float_output.h"
#include "esphome/components/sensor/sensor.h"
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

class MCP4661Component;

class MCP4661Channel : public Parented<MCP4661Component> {
  public:
    void set_wiper(uint8_t wiper) { wiper_ = wiper; update_wiper_address(); }
    void set_volatility(bool is_volatile) { is_volatile_ = is_volatile; update_wiper_address(); }
    uint8_t get_wiper(void) { return wiper_; }
    bool get_volatility(void) { return is_volatile_; }

  protected:
    friend class MCP4661Component;
    void update_wiper_address(void); 

  uint8_t wiper_;
  bool is_volatile_;
  MemoryAddress wiper_address_;
  uint16_t wiper_value_max_;
  float wiper_step_size_;
}

class MCP4661SensorChannel : public MCP4661Channel, public PollingComponent, public sensor::Sensor {
  public:
    void update(void) override;
};

class MCP4661OutputChannel : pubic MCP4661Channel, public Component, public output::FloatOutput {
 protected:
  void write_state(float state) override;
};

class MCP4661Component : public Component, public i2c::I2CDevice {
 public:
  MCP4661Component() {}

  void register_channel(MCP4661Channel * channel);

  void set_number_of_bits(int bits) { number_of_bits_ = bits; }
  void set_number_of_wipers(int wipers) { number_of_wipers_ = wipers; }

  void setup() override;
  void dump_config() override;

 protected:
  friend class MCP4661Channel;
  friend class MCP4661OutputChannel;
  friend class MCP4661SensorChannel;

  enum ErrorCode { NONE = 0, COMMUNICATION_FAILED } error_code_{NONE};
  
  uint8_t get_wiper_value(MemoryAddress wiper_address, uint16_t value);
  void set_wiper_value(MemoryAddress wiper_address, uint16_t value);
  uint16_t get_wiper_value(MemoryAddress wiper_address);
  static uint8_t construct_command_byte(MemoryAddress memory_address, Command command, uint16_t data);

  int number_of_bits_, number_of_wipers_;
  float wiper_step_size_;
  uint16_t wiper_value_max_;
};

}  // namespace mcp4661
}  // namespace esphome
