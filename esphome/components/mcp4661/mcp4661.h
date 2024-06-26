#pragma once

#include "esphome/components/output/float_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/i2c/i2c.h"

#include <vector>

namespace esphome {
namespace mcp4661 {

enum MemoryAddress {
  VOLATILE_WIPER_0 = 0x00,
  VOLATILE_WIPER_1 = 0x01,
  NON_VOLATILE_WIPER_0 = 0x02,
  NON_VOLATILE_WIPER_1 = 0x03,
};

enum Command {
  WRITE = 0x00,
  INCREMENT = 0x01,
  DECREMENT = 0x02,
  READ = 0x03,
};

enum MCP4661SensorType {
  WIPER,
  MEMORY,
};

enum MCP4661MemoryLocation {
  TCON = 0x04,
  STATUS = 0x05,
  DATA_0 = 0x06,
  DATA_1 = 0x07,
  DATA_2 = 0x08,
  DATA_3 = 0x09,
  DATA_4 = 0x0A,
  DATA_5 = 0x0B,
  DATA_6 = 0x0C,
  DATA_7 = 0x0D,
  DATA_8 = 0x0E,
  DATA_9 = 0x0F,
};

class MCP4661Component;

// Ideally there would be an abstract subclass for both channel types but the Parented<> subclass
// appears to make this impossible.

class MCP4661SensorChannel : public PollingComponent, public sensor::Sensor {
  friend class MCP4661Component;

  public:
    MCP4661SensorChannel(MCP4661Component * parent) : parent_(parent) {}
    void set_type(MCP4661SensorType type) { type_ = type; }
    void set_channel(uint8_t wiper) { wiper_ = wiper; }
    void set_volatility(bool is_volatile) { is_volatile_ = is_volatile; }
    void set_location(uint8_t location) { location_ = location; }
    MCP4661SensorType get_type(void) { return type_; }
    uint8_t get_channel(void) { return wiper_; }
    bool get_volatility(void) { return is_volatile_; }
    uint8_t get_location(void) { return location_; }
    void update(void) override;

  protected:
    MCP4661SensorType type_;
    bool is_volatile_;
    uint8_t wiper_, location_;
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

    MemoryAddress calculate_memory_address(uint8_t wiper, bool is_volatile);
    uint8_t construct_command_byte(uint8_t memory_address, Command command, uint16_t data);
    uint16_t get_memory_value(uint8_t memory_address);
    uint16_t get_wiper_value(uint8_t wiper, bool is_volatile);
    void set_wiper_value(uint8_t wiper, bool is_volatile, uint16_t value);

    float wiper_value_max_;
    uint8_t number_of_bits_, number_of_wipers_;

    std::vector<MCP4661SensorChannel*> sensors_;
    std::vector<MCP4661OutputChannel*> outputs_;
};

}  // namespace mcp4661
}  // namespace esphome
