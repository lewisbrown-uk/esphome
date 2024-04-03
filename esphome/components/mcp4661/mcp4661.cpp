#include "mcp4661.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace mcp4661 {

static const char *const TAG = "mcp4661";

static uint8_t MCP4661Component::calculate_wiper_address(uint8_t wiper, bool is_volatile) { 
  const MemoryAddress volatile_addresses[] = { MemoryAddress::VOLATILE_WIPER_0, MemoryAddress::VOLATILE_WIPER_1 };
  const MemoryAddress non_volatile_addresses[] = { MemoryAddress::NON_VOLATILE_WIPER_0, MemoryAddress::NON_VOLATILE_WIPER_1 };
  uint8_t wiper_address;

  if (is_volatile) {
    wiper_address = volatile_addresses[wiper];
  }
  else {
    wiper_address = non_volatile_addresses[wiper];
  }

  ESP_LOGD(TAG, "Wiper address = %02x, wiper = %01u, volatile = %01u",
    wiper_address, wiper, is_volatile);
  
  return wiper_address;
}

static uint8_t MCP4661Component::construct_command_byte(uint8_t wiper, bool is_volatile, Command command, uint16_t data) {
  memory_address = this->calculate_wiper_address(wiper, is_volatile);
  return (memory_address << 4) | (command << 2) | ((data & 0x1ff) >> 8);
}

void MCP4661Component::dump_config(void) {
  // dump config
  ESP_LOGCONFIG(TAG, "bits = %01u, wiper_step_size = %f, wiper_value_max = %01u", 
    this->number_of_bits_, 
    this->wiper_step_size_,
    this->wiper_value_max_);
  ESP_LOGCONFIG(TAG, "wiper channels = %01u", this->number_of_wipers_);

  for (auto sensor : this->sensors_) {
    LOG_SENSOR("  ", "SENSOR", sensor);
    ESP_LOGCONFIG(TAG, "    channel: %u", sensor->get_channel());
    ESP_LOGCONFIG(TAG, "    volatile: %u", sensor->get_volatility());
  }

  for (auto output : this->outputs_) {
    ESP_LOGCONFIG(TAG, "MCP4661 output");
    ESP_LOGCONFIG(TAG, "    channel: %u", sensor->get_channel());
    ESP_LOGCONFIG(TAG, "    volatile: %u", sensor->get_volatility());
  }
}

void MCP4661Component::setup(void) {
  ESP_LOGCONFIG(TAG, "Setting up MCP4661");
  // set up the MCP4661
  if ( this->number_of_bits_ == 8 )  
  {
    this->wiper_step_size_ = 1.0/256.0;
    this->wiper_value_max_ = 0x100;
  }
  else if ( this->number_of_bits_ == 7 )
  {
    this->wiper_step_size_ = 1.0/128.0;
    this->wiper_value_max_ = 0x80;
  }
  else
  {
    // This is an error - should not be possible because of validation
    ESP_LOGV(TAG, "Invalid number of bits specifed: %u", this->number_of_bits_);
    assert(false); // TODO: throw an error properly
  }
}

void MCP4661Component::set_wiper_value(uint8_t wiper, bool is_volatile, uint16_t wiper_value) {
  uint8_t command_byte = construct_command_byte(wiper, is_volatile, Command::WRITE, wiper_value);
  uint8_t data_byte = wiper_value & 0xff;

  ESP_LOGD(TAG, "Writing bytes %02x %02x for wiper %u, volatile %u command %02x, value %02x", 
    command_byte, data_byte, wiper, is_volatile, Command::WRITE, wiper_value);

  this->write_byte(command_byte, data_byte);
}

uint16_t MCP4661Component::get_wiper_value(uint8_t wiper, bool is_volatile) {
  uint8_t command_byte = construct_command_byte(wiper, is_volatile, Command::READ, 0);
  uint16_t value;

  this->read_byte_16(command_byte, &value);
  ESP_LOGD(TAG, "wiper %u volatile %u value = %04x", wiper, is_volatile, value);
  return value;
}

void MCP4661OutputChannel::write_state(float state) {
  uint16_t wiper_value = 0;
  if ( state == 1.0 )
  {
    wiper_value = this->parent_->wiper_value_max_;
  }
  wiper_value = uint16_t(state * this->parent_->wiper_step_size_);
  ESP_LOGD(TAG, "state = %f wiper_value = %02x", state, wiper_value);
  // write state
  this->parent_->set_wiper_value(this->wiper_, this->is_volatile_, wiper_value);
}

void MCP4661SensorChannel::update(void) {
  uint16_t wiper_value = this->parent_->get_wiper_value(this->wiper_, this->is_volatile_);
  this->publish_state(wiper_value);
}

}  // namespace mcp4661
}  // namespace esphome
