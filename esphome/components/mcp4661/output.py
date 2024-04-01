import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_CHANNEL, CONF_ID
from . import MCP4661Output, mcp4661_ns

DEPENDENCIES = ["mcp4661"]

MCP4661Channel = mcp4661_ns.class_("MCP4661Channel", output.FloatOutput)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(MCP4661Channel),
        cv.GenerateID(CONF_MCP4661_ID): cv.use_id(MCP4661Output),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=1),
        cv.Optional(CONF_VOLATILE, default=True): cv.boolean
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_MCP4661_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.set_volatility(config[CONF_VOLATILE]))
    cg.add(paren.register_channel(var))
    await output.register_output(var, config)
    return var
