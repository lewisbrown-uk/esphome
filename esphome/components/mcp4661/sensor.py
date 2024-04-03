import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, voltage_sampler
from esphome.const import CONF_CHANNEL, CONF_ID
from . import MCP4661Component, mcp4661_ns, CONF_MCP4661_ID, CONF_VOLATILE

DEPENDENCIES = ["mcp4661"]

MCP4661SensorChannel = mcp4661_ns.class_("MCP4661SensorChannel", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = (
    sensor.sensor_schema(MCP4661SensorChannel, accuracy_decimals=0).extend(
        {
            cv.GenerateID(CONF_MCP4661_ID): cv.use_id(MCP4661Component),
            cv.Required(CONF_VOLATILE): cv.boolean,
            cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=1),
        }
    )
    .extend(cv.polling_component_schema("1s"))
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_MCP4661_ID])
    var = cg.new_Pvariable(config[CONF_ID], paren)
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)

    cg.add(var.set_volatility(config[CONF_VOLATILE]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))

    cg.add(paren.register_sensor_channel(var))
