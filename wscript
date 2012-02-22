from os import system, remove
from os.path import abspath, exists, join
from shutil import rmtree
import Utils

srcdir = abspath('.')
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

def clean(ctx):
  if exists('build'): rmtree('build')

def build_post(bld):
  module_path = bld.path.find_resource('bsdiff.node').abspath(bld.env)
  system('mkdir -p lib')
  system('cp %r lib/bsdiff.node' % module_path)

def build(bld):
  node_bsdiff = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  node_bsdiff.source = ['src/cpp/binding.cc', 'src/cpp/bsdiff.cc']
  node_bsdiff.target = 'bsdiff'
  node_bsdiff.cxxflags = ['-Wall', '-g']

  bld.add_post_fun(build_post)
