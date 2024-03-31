#include "mcp4661.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace mcp4661 {

static const char *const TAG = "mcp4661";

void MCP4661Output::setup(void)
{
  ESP_LOGCONFIG(tag, "Setting up MCP4661");
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
    ESP_LOGV(tag, "Invalid number of bits specifed: %01u", this->number_of_bits_);
    assert(false); // TODO: throw an error properly
  }
}

void MCP4661Output::dump_config(void)
{
  // dump config
}

void MCP4661Output::register_channel(MCP4661Channel *channel) {
  auto c = channel->wiper_;
  this->min_channel_ = std::min(this->min_channel_, c);
  this->max_channel_ = std::max(this->max_channel_, c);
  channel->set_parent(this);
  channel->wiper_step_size_ = this->wiper_step_size_;
  channel->wiper_value_max_ = this->wiper_value_max_;
  ESP_LOGV(TAG, "Registered channel: %01u", c);
}

void MCP4661Output::set_wiper_value(MemoryAddress wiper_address, uint16_t wiper_value)
{
  uint8_t command_byte = construct_command_byte(wiper_address, Command::WRITE, wiper_value);
  uint8_t data_byte = wiper_value & 0xff;

}

void MCP4661Channel::write_state(float state)
{
  uint16_t wiper_value = 0;
  if ( state == 1.0 )
  {
    wiper_value = this->wiper_value_max_;
  }
  wiper_value = uint16_t(state * this->wiper_step_size_);

  // write state
  this->parent_->set_wiper_value(this->wiper_address_, wiper_value);
}

uint8_t MCP4661Output::construct_command_byte(MemoryAddress memory_address, Command command, uint16_t data)
{
  return (memory_address << 4) | (command << 2) | ((data & 0x1ff) >> 8);
}

}  // namespace mcp4661
}  // namespace esphome
