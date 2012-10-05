def set_options(opt):
    opt.tool_options("compiler_cxx")

def configure(conf):
    conf.check_tool("compiler_cxx")
    conf.check_tool("node_addon")
    conf.check_cfg(package='alsa', args='--cflags --libs', uselib_store='LIBALSA')

def build(bld):
    obj = bld.new_task_gen("cxx", "shlib", "node_addon")
    obj.cxxflags = ["-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
    obj.uselib = ['LIBALSA', 'LIBM']
    obj.target = "oscillator"
    obj.source = "main.cpp"
