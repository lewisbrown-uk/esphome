import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_CHANNEL, CONF_ID
from . import MCP4661Component, mcp4661_ns, CONF_MCP4661_ID, CONF_VOLATILE

DEPENDENCIES = ["mcp4661"]

MCP4661OutputChannel = mcp4661_ns.class_("MCP4661OutputChannel", output.FloatOutput)

CONFIG_SCHEMA = (
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_MCP4661_ID): cv.use_id(MCP4661Component),
            cv.Required(CONF_ID): cv.declare_id(MCP4661OutputChannel),
            cv.Required(CONF_VOLATILE): cv.boolean,
            cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=1),
        }
    )
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_MCP4661_ID])
    var = cg.new_Pvariable(config[CONF_ID], paren)
    await output.register_output(var, config)
    await cg.register_component(var, config)

    cg.add(var.set_volatility(config[CONF_VOLATILE]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))

    cg.add(paren.register_output_channel(var))
