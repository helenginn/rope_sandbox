project('switch', 'cpp', 'c')
qt5 = import('qt5')

qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'], required : true)
dep_vgeom = dependency('libvgeom', required: false)
dep_gl = dependency('gl', required : true)
dep_c4x = dependency('libcluster4x')
png_dep = dependency('libpng')
helen3d = dependency('helen3d')
helencore = dependency('helencore')
vagabond = dependency('libvag')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

# Run the version and hash generator
hash_script = find_program('get_hash.sh')

gen_src = custom_target('commit-output',
                        input : ['src/commit.h.pre'],
                        output : ['commit.h'],
                        command : [hash_script, '@INPUT@', '@OUTPUT@'],
                        build_by_default: true,
                        build_always_stale: true)

# Graphical user interface
# (including all the Qt and OpenGL stuff)

# Pre-processing by Qt5

moc_files = qt5.preprocess(moc_headers : [
'src/Composite.h', 
'src/Display.h', 
'src/Handle.h', 
'src/Handleable.h', 
'src/MyDictator.h', 
],
		moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'])

executable('switch', gen_src, moc_files,
'src/CAlpha.cpp', 
'src/Chain.cpp', 
'src/Composite.cpp', 
'src/Display.cpp', 
'src/Ensemble.cpp', 
'src/Entity.cpp', 
'src/Handle.cpp', 
'src/Handleable.cpp', 
'src/Kabsch.cpp', 
'src/MyDictator.cpp', 
'src/StructureView.cpp', 
'src/main.cpp', 
cpp_args: ['-std=c++11'], dependencies : [ 
qt5_dep, 
dep_gl, 
dep_c4x, 
dep_vgeom,
png_dep,
helen3d,
helencore,
vagabond,
], install: true)

