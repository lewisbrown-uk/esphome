import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, voltage_sampler
from esphome.const import CONF_CHANNEL, CONF_ID, CONF_TYPE
from . import MCP4661Component, mcp4661_ns, CONF_MCP4661_ID, CONF_VOLATILE, CONF_LOCATION

DEPENDENCIES = ["mcp4661"]

MCP4661SensorChannel = mcp4661_ns.class_("MCP4661SensorChannel", sensor.Sensor, cg.PollingComponent)

MCP4661SensorType = mcp4661_ns.enum("MCP4661SensorType")
SENSOR_TYPE = {
    "WIPER" : MCP4661SensorType.WIPER,
    "MEMORY" : MCP4661SensorType.MEMORY,
}

MCP4661MemoryLocation = mcp4661_ns.enum("MCP4661MemoryLocation")
MEMORY_LOCATION = {
    "TCON" : MCP4661MemoryLocation.TCON,
    "STATUS" : MCP4661MemoryLocation.STATUS,
    "DATA_0" : MCP4661MemoryLocation.DATA_0,
    "DATA_1" : MCP4661MemoryLocation.DATA_1,
    "DATA_2" : MCP4661MemoryLocation.DATA_2,
    "DATA_3" : MCP4661MemoryLocation.DATA_3,
    "DATA_4" : MCP4661MemoryLocation.DATA_4,
    "DATA_5" : MCP4661MemoryLocation.DATA_5,
    "DATA_0" : MCP4661MemoryLocation.DATA_6,
    "DATA_7" : MCP4661MemoryLocation.DATA_7,
    "DATA_8" : MCP4661MemoryLocation.DATA_8,
    "DATA_9" : MCP4661MemoryLocation.DATA_9,
    "DATA_10" : MCP4661MemoryLocation.DATA_10,
}

# accuracy_decimals = 3 because 1/256.0 = 0.00390625 so 3 decimals are required for precision
CONFIG_SCHEMA = (
    sensor.sensor_schema(MCP4661SensorChannel, accuracy_decimals=3).extend(
        {
            cv.GenerateID(CONF_MCP4661_ID): cv.use_id(MCP4661Component),
            cv.Required(CONF_TYPE): cv.enum(SENSOR_TYPE, upper=True),
            cv.Optional(CONF_VOLATILE, default=True): cv.boolean,
            cv.Optional(CONF_CHANNEL, default=0): cv.int_range(min=0, max=1),
            cv.Optional(CONF_LOCATION, default="STATUS"): cv.enum(MEMORY_LOCATION, upper=True, space="_"),
        }
    )
    .extend(cv.polling_component_schema("1s"))
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_MCP4661_ID])
    var = cg.new_Pvariable(config[CONF_ID], paren)
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)

    cg.add(var.set_type(config[CONF_TYPE]))
    cg.add(var.set_volatility(config[CONF_VOLATILE]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.set_location(config[CONF_LOCATION]))

    cg.add(paren.register_sensor_channel(var))
