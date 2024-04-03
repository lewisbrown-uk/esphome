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

// Ideally there would be an abstract subclass for both channel types but the Parented<> subclass
// appears to make this impossible.

class MCP4661SensorChannel : public PollingComponent, public sensor::Sensor {
  friend class MCP4661Component;

  public:
    MCP4661SensorChannel(MCP4661Component * parent);
    void set_channel(uint8_t wiper) { wiper_ = wiper; update_wiper_address(); }
    void set_volatility(bool is_volatile) { is_volatile_ = is_volatile; update_wiper_address(); }
    uint8_t get_channel(void) { return wiper_; }
    bool get_volatility(void) { return is_volatile_; }
    void update(void) override;
    void set_parent(MCP4661Component * parent);

  protected:
    void update_wiper_address(void);

    uint8_t wiper_;
    bool is_volatile_;
    MemoryAddress wiper_address_;
    uint16_t wiper_value_max_;
    float wiper_step_size_;
    MCP4661Component * parent_;
};

class MCP4661OutputChannel : public Component, public output::FloatOutput {
  friend class MCP4661Component;

  public:
    MCP4661OutputChannel(MCP4661Component * parent);
    void set_channel(uint8_t wiper) { wiper_ = wiper; update_wiper_address(); }
    void set_volatility(bool is_volatile) { is_volatile_ = is_volatile; update_wiper_address(); }
    uint8_t get_channel(void) { return wiper_; }
    bool get_volatility(void) { return is_volatile_; }
    void set_parent(MCP4661Component * parent);

  protected:
    void write_state(float state) override;
    void update_wiper_address(void);

    uint8_t wiper_;
    bool is_volatile_;
    MemoryAddress wiper_address_;
    uint16_t wiper_value_max_;
    float wiper_step_size_;
    MCP4661Component * parent_;
};

class MCP4661Component : public Component, public i2c::I2CDevice {

  friend class MCP4661OutputChannel;
  friend class MCP4661SensorChannel;

  public:
    MCP4661Component() {}

    void register_sensor_channel(MCP4661SensorChannel * channel);
    void register_output_channel(MCP4661OutputChannel * channel);

    void set_number_of_bits(int bits) { number_of_bits_ = bits; }
    void set_number_of_wipers(int wipers) { number_of_wipers_ = wipers; }

    void setup() override;
    void dump_config() override;

  protected:

    enum ErrorCode { NONE = 0, COMMUNICATION_FAILED } error_code_{NONE};
    
    void set_wiper_value(MemoryAddress wiper_address, uint16_t value);
    uint16_t get_wiper_value(MemoryAddress wiper_address);
    static uint8_t construct_command_byte(MemoryAddress memory_address, Command command, uint16_t data);

    int number_of_bits_, number_of_wipers_;
    float wiper_step_size_;
    uint16_t wiper_value_max_;
};

}  // namespace mcp4661
}  // namespace esphome
