#include "mcp4661.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace mcp4661 {

static const char *const TAG = "mcp4661";

/**
 * Calculate memory address based on wiper index and volatility.
 * 
 * @param[in] wiper The wiper index
 * @param[in] is_volatile Indicates whether to address the volatile or non-volatile wiper
 * 
 */

MemoryAddress MCP4661Component::calculate_memory_address(uint8_t wiper, bool is_volatile) { 
  const MemoryAddress volatile_addresses[] = { MemoryAddress::VOLATILE_WIPER_0, MemoryAddress::VOLATILE_WIPER_1 };
  const MemoryAddress non_volatile_addresses[] = { MemoryAddress::NON_VOLATILE_WIPER_0, MemoryAddress::NON_VOLATILE_WIPER_1 };
  MemoryAddress memory_address;

  if (is_volatile) {
    memory_address = volatile_addresses[wiper];
  }
  else {
    memory_address = non_volatile_addresses[wiper];
  }

  ESP_LOGD(TAG, "Wiper address = %02x, wiper = %01u, volatile = %01u",
    memory_address, wiper, is_volatile);
  
  return memory_address;
}

/**
 * Construct the command byte for I2C transactions.
 * 
 * @param[in] memory_address The memory address to read or write
 * @param[in] command The command (write = 0, read = 3)
 * @param[in] data The data for the write command, ignored for other commands
 * 
 * The format of the command byte is AAAACCxD where:
 * 
 *    A = address bit
 *    C = command bit
 *    D = bit 8 of data
 */

uint8_t MCP4661Component::construct_command_byte(uint8_t memory_address, Command command, uint16_t data) {
  if ( command == Command::WRITE )
    return (memory_address << 4) | (command << 2) | ((data & 0x1ff) >> 8);
  else
    return (memory_address << 4) | (command << 2);
}

/**
 * Dump configuration.
 */

void MCP4661Component::dump_config(void) {
  // dump config
  ESP_LOGCONFIG(TAG, "MCP4661 config");

  LOG_I2C_DEVICE(this);

  ESP_LOGCONFIG(TAG, "bits = %01u, wiper_value_max = %01u", this->number_of_bits_, this->wiper_value_max_);
  ESP_LOGCONFIG(TAG, "wiper channels = %01u", this->number_of_wipers_);
/*
  for (auto *sensor : this->sensors_) {
    LOG_SENSOR("  ", "SENSOR", sensor);
    ESP_LOGCONFIG(TAG, "    channel: %u", sensor->get_channel());
    ESP_LOGCONFIG(TAG, "    volatile: %u", sensor->get_volatility());
  }

  for (auto *output : this->outputs_) {
    ESP_LOGCONFIG(TAG, "MCP4661 output");
    ESP_LOGCONFIG(TAG, "    channel: %u", sensor->get_channel());
    ESP_LOGCONFIG(TAG, "    volatile: %u", sensor->get_volatility());
  }*/
}

/**
 * Set up the MCP4661 component. No hardware setup is required.
 */

void MCP4661Component::setup(void) {
  ESP_LOGCONFIG(TAG, "Setting up MCP4661");
  // set up the MCP4661
  if ( this->number_of_bits_ == 8 )  
  {
    this->wiper_value_max_ = 256.0;
  }
  else if ( this->number_of_bits_ == 7 )
  {
    this->wiper_value_max_ = 128.0;
  }
  else
  {
    // This is an error - should not be possible because of validation
    ESP_LOGV(TAG, "Invalid number of bits specifed: %u", this->number_of_bits_);
    assert(false); // TODO: throw an error properly
  }
}

/**
 * Sets the value of a specified wiper to a specified value.
 * @param[in] wiper The wiper index (0 or 1)
 * @param[in] is_volatile Indicates whether to set the volatile or non-volatile wiper of this index
 * @param[in] wiper_value The value to set, in the range 0-128 or 0-256 depending on the device (note 9 bits max.)
 * 
 * The I2C transaction to write is: 
 * 
 * Host: [address|write] [command byte] [data byte]
 * 
 * where [command byte] contains the memory address to write in the high 4 bits, the command (0=write) in bits 2-3, 
 * and the high bit of the 9-bit data value in bit 0. [data byte] contains the bottom 8 bits of the value.
 */

void MCP4661Component::set_wiper_value(uint8_t wiper, bool is_volatile, uint16_t wiper_value) {
  MemoryAddress memory_address = this->calculate_memory_address(wiper, is_volatile);
  uint8_t command_byte = construct_command_byte(static_cast<uint8_t>(memory_address), Command::WRITE, wiper_value);
  uint8_t data_byte = wiper_value & 0xff;

  ESP_LOGD(TAG, "Writing bytes %02x %02x for wiper %u, volatile %u, command %02x, value %02x", 
    command_byte, data_byte, wiper, is_volatile, Command::WRITE, wiper_value);

  // Send the command and data bytes with a stop bit
  this->write_byte(command_byte, data_byte, true);
}

/**
 * Reads the value of a specified memory location.
 * 
 * @param[in] location The memory location to read (0x0 - 0xF)
 * @return The 8- or 9-bit value, depending on the device, as a uint16_t
 * 
 * The I2C transaction to read is:
 * 
 * Host:   [address|write] [command byte] [address|read]
 * Device: [data MSB] [data LSB]
 * 
 * where [command byte] contains the memory address to read in the high 4 bits, the command (3=read) in bits 2-3.
 * The device returns the 8- or 9-bit value of the location addressed by the command byte in the following two frames.
 * The MSB and LSB are OR-ed together to produce a 16-bit return value.
 * 
 */

uint16_t MCP4661Component::get_memory_value(uint8_t location) {
  uint8_t command_byte = construct_command_byte(location, Command::READ, 0);
  uint8_t bytes[2];
  uint16_t value;

  // Send the command byte without a stop bit
  this->write(&command_byte, 1, false);
  this->read(bytes, 2);
  value = (bytes[0]<<8) | bytes[1];
  ESP_LOGD(TAG, "location %u command %02x value = %04x", location, command_byte, value);
  return value;
}

/**
 * Reads the value of a specified wiper.
 * 
 * @param[in] wiper The wiper index (0 or 1)
 * @param[in] is_volatile Indicates whether to read the volatile or non-volatile wiper of this index
 * @return The 8- or 9-bit value, depending on the device as a uint16_t
 * 
 */

uint16_t MCP4661Component::get_wiper_value(uint8_t wiper, bool is_volatile) {
  MemoryAddress memory_address = this->calculate_memory_address(wiper, is_volatile);
  return this->get_memory_value(static_cast<uint8_t>(memory_address));
}

/**
 * Convert the floating point state of the output to a wiper value and send.
 * 
 * @param[in] state A floating point number between 0.0 and 1.0
 * 
 * If state is exactly 1.0 the maximum wiper value is sent (128 for 7-bit devices and 256 for 8-bit devices).
 * Note that these values have one more bit than the nominal number of bits of the device.
 * 
 * If state is other than 1.0 the state is converted to a wiper value based on the number of bits of the device.  
 */
void MCP4661OutputChannel::write_state(float state) {
  uint16_t wiper_value = 0;
  if ( state == 1.0 )
  {
    wiper_value = this->parent_->wiper_value_max_;
  }
  wiper_value = uint16_t(state * this->parent_->wiper_value_max_);
  ESP_LOGD(TAG, "state = %f wiper_value = %02x", state, wiper_value);
  // write state
  this->parent_->set_wiper_value(this->wiper_, this->is_volatile_, wiper_value);
}

/**
 * Update the sensor value with the actual value of the relevant memory location.
 */

void MCP4661SensorChannel::update(void) {
  if ( this->type_ == MCP4661SensorType::WIPER ) {
    uint16_t wiper_value = this->parent_->get_wiper_value(this->wiper_, this->is_volatile_);
    this->publish_state(float(wiper_value) / this->parent_->wiper_max_value_);
  }
  else if ( this->type_ == MCP4661SensorType::MEMORY ) {
    uint16_t memory_value = this->parent_->get_memory_value(this->location_);
    this->publish_state(memory_value);
  }
  else
  {
    ESP_LOGD(TAG, "Invalid value for type");
  }
}

}  // namespace mcp4661
}  // namespace esphome
