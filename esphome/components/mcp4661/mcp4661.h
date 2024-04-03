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
    MCP4661SensorChannel(MCP4661Component * parent) : parent_(parent) {}
    void set_channel(uint8_t wiper) { wiper_ = wiper; }
    void set_volatility(bool is_volatile) { is_volatile_ = is_volatile; }
    uint8_t get_channel(void) { return wiper_; }
    bool get_volatility(void) { return is_volatile_; }
    void update(void) override;

  protected:

    bool is_volatile_;
    uint8_t wiper_;
    MCP4661Component * parent_;
};

class MCP4661OutputChannel : public Component, public output::FloatOutput {
  friend class MCP4661Component;

  public:
    MCP4661OutputChannel(MCP4661Component * parent) : parent_(parent) {}
    void set_channel(uint8_t wiper) { wiper_ = wiper; }
    void set_volatility(bool is_volatile) { is_volatile_ = is_volatile; }
    uint8_t get_channel(void) { return wiper_; }
    bool get_volatility(void) { return is_volatile_; }

  protected:
    void write_state(float state) override;

    bool is_volatile_;
    uint8_t wiper_;
    MCP4661Component * parent_;
};

class MCP4661Component : public Component, public i2c::I2CDevice {

  friend class MCP4661OutputChannel;
  friend class MCP4661SensorChannel;

  public:
    void register_sensor_channel(MCP4661SensorChannel * channel) { sensors_.push_back(channel); }
    void register_output_channel(MCP4661OutputChannel * channel) { outputs_.push_back(channel); }

    void set_number_of_bits(uint8_t bits) { number_of_bits_ = bits; }
    void set_number_of_wipers(uint8_t wipers) { number_of_wipers_ = wipers; }

    void setup() override;
    void dump_config() override;

  protected:

    static uint8_t calculate_memory_address(uint8_t wiper, bool is_volatile);
    static uint8_t construct_command_byte(uint8_t wiper, bool is_volatile, Command command, uint16_t data);
    uint16_t get_wiper_value(uint8_t wiper, bool is_volatile);
    void set_wiper_value(uint8_t wiper, bool is_volatile, uint16_t value);

    float wiper_step_size_;
    uint8_t number_of_bits_, number_of_wipers_;
    uint16_t wiper_value_max_;

    std::vector<MCP4661SensorChannel*> sensors_;
    std::vector<MCP4661OutputChannel*> outputs_;
};

}  // namespace mcp4661
}  // namespace esphome
