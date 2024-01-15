import esphome.codegen as cg

nvs_ns = cg.esphome_ns.namespace("nvs_flash")
NVSFlash = nvs_ns.class_("NvsFlash")
NVSFlashConstRef = NVSFlash.operator("ref").operator("const")


CONFIG_SCHEMA = {}


async def to_code(config):
    # cg.add_global(nvs_ns.using)
    pass
