import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

AUTO_LOAD = ["output"]
CODEOWNERS = ["@lewisbrown-uk"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

mcp4661_ns = cg.esphome_ns.namespace("mcp4661")
MCP4661Output = mcp4661_ns.class_("MCP4661Output", cg.Component, i2c.I2CDevice)
CONF_BITS = "bits"
CONF_WIPERS = "wipers"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MCP4661Output),
            cv.Optional(CONF_BITS, default=8): cv.int_range(min=7, max=8),
            cv.Optional(CONF_WIPERS, default=2): cv.int_range(min=1, max=2),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x2f))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_number_of_bits(config[CONF_BITS]))
    cg.add(var.set_number_of_wipers(config[CONF_WIPERS]))
    await i2c.register_i2c_device(var, config)
    return var
