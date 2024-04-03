#include "mcp4661.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace mcp4661 {

static const char *const TAG = "mcp4661";

uint8_t MCP4661Component::construct_command_byte(MemoryAddress memory_address, Command command, uint16_t data) {
  return (memory_address << 4) | (command << 2) | ((data & 0x1ff) >> 8);
}

void MCP4661Component::dump_config(void) {
  // dump config
  ESP_LOGCONFIG(TAG, "bits = %01u, wiper_step_size = %f, wiper_value_max = %01u", 
    this->number_of_bits_, 
    this->wiper_step_size_,
    this->wiper_value_max_);
  ESP_LOGCONFIG(TAG, "wiper channels = %01u", this->number_of_wipers_);
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
    ESP_LOGV(TAG, "Invalid number of bits specifed: %01u", this->number_of_bits_);
    assert(false); // TODO: throw an error properly
  }
}

void MCP4661SensorChannel::register_parent(MCP4661Component * parent) {
  auto c = this->wiper_;
  this->set_parent(parent);
  this->wiper_step_size_ = parent->wiper_step_size_;
  this->wiper_value_max_ = parent->wiper_value_max_;
  ESP_LOGD(TAG, "Registered sensor channel: %01u", c);
  if (c > parent->number_of_wipers_ - 1) {
    ESP_LOGW(TAG, "Channel number is out of range for this device");
  }
}

void MCP4661OutputChannel::register_parent(MCP4661Component * parent) {
  auto c = this->wiper_;
  this->set_parent(parent);
  this->wiper_step_size_ = parent->wiper_step_size_;
  this->wiper_value_max_ = parent->wiper_value_max_;
  ESP_LOGD(TAG, "Registered output channel: %01u", c);
  if (c > parent->number_of_wipers_ - 1) {
    ESP_LOGW(TAG, "Channel number is out of range for this device");
  }
}

void MCP4661Component::set_wiper_value(MemoryAddress wiper_address, uint16_t wiper_value) {
  uint8_t command_byte = construct_command_byte(wiper_address, Command::WRITE, wiper_value);
  uint8_t data_byte = wiper_value & 0xff;

  ESP_LOGD(TAG, "Writing bytes %02x %02x for address %02x, command %02x, value %02x", 
    command_byte, data_byte, wiper_address, Command::WRITE, wiper_value);

  this->write_byte(command_byte, data_byte);
}

uint16_t MCP4661Component::get_wiper_value(MemoryAddress wiper_address) {
  uint8_t command_byte = construct_command_byte(wiper_address, Command::READ, 0);
  uint16_t value;

  this->read_byte_16(command_byte, &value);
  return value;
}

void MCP4661SensorChannel::update_wiper_address(void) { 
  this->wiper_address_ = MemoryAddress((this->is_volatile_?VOLATILE_WIPER_0:NON_VOLATILE_WIPER_0) + this->wiper_); 
  ESP_LOGD(TAG, "Update wiper address to %02x, wiper = %01u, volatile = %01u",
    this->wiper_address_, this->wiper_, this->is_volatile_);
}

void MCP4661OutputChannel::update_wiper_address(void) { 
  this->wiper_address_ = MemoryAddress((this->is_volatile_?VOLATILE_WIPER_0:NON_VOLATILE_WIPER_0) + this->wiper_); 
  ESP_LOGD(TAG, "Update wiper address to %02x, wiper = %01u, volatile = %01u",
    this->wiper_address_, this->wiper_, this->is_volatile_);
}

void MCP4661OutputChannel::write_state(float state) {
  uint16_t wiper_value = 0;
  if ( state == 1.0 )
  {
    wiper_value = this->wiper_value_max_;
  }
  wiper_value = uint16_t(state * this->wiper_step_size_);
  ESP_LOGD(TAG, "state = %f wiper_value = %02x", state, wiper_value);
  // write state
  this->parent_->set_wiper_value(this->wiper_address_, wiper_value);
}

void MCP4661SensorChannel::update(void) {
  uint16_t wiper_value = this->parent_->get_wiper_value(this->wiper_address_);
  this->publish_state(wiper_value);
}

}  // namespace mcp4661
}  // namespace esphome
