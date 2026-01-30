#! /usr/bin/env python3

import os, sys, subprocess, platform, argparse, shutil, multiprocessing

def removeBase(s, baseDir):
  p = s.find(baseDir)
  if p>=0:
    s = s[0:p] + '.' + s[p + len(baseDir):]
  return s

def printCommand(cmd, baseDir):
  if not cmd:
    return
    
  print('____________________________')
  print(removeBase(cmd[0], baseDir))
  for w in cmd[1:]:
    if w[0] == '-':
      print('\t' + removeBase(w, baseDir))
    else:
      print('\t\t' + removeBase(w, baseDir))
  print('____________________________')
     
def init():
  p = platform.system()
  if p == 'Windows':
      defaultCompiler = 'msvc'
      gen = '-GNinja'
  else:
      defaultCompiler = 'gnu'
      gen = '-GUnix Makefiles'

  parser = argparse.ArgumentParser()
  parser.add_argument('-d', '--debug', action='store_true')
  parser.add_argument('-c', '--compiler', default=defaultCompiler)
  parser.add_argument('--float', action='store_true')
  
  args = parser.parse_args()
  args.baseDir = os.getcwd()
    
  if p == 'Windows':
      args.cmake_gen = '-GNinja'
  else:
      args.cmake_gen = '-GUnix Makefiles'
      
  args.env = os.environ.copy()
  if args.compiler == 'gnu':
    args.env['CC'] = 'gcc'
    args.env['CXX'] = 'g++'
  elif args.compiler == 'intel':
    args.env['CC'] = 'icx'
    args.env['CXX'] = 'icpx'
  elif args.compiler == 'msvc':
    args.env['CC'] = 'cl.exe'
    args.env['CXX'] = 'cl.exe'
  
  if args.debug:
     args.mode = "debug"
  else:
     args.mode = "release"

  args.cmake_params = []
  return args

def build(args, subDir):

  if subDir:
    baseDir = os.path.join(args.baseDir, subDir)
  else:
    baseDir = args.baseDir
  print (baseDir)
    
  srcDir = os.path.join(baseDir, 'src')
  
  if hasattr(args, 'backend'):
    buildDir = os.path.join(baseDir, 'build', args.compiler, args.backend, args.mode)
    installDir = os.path.join(baseDir, 'install', args.compiler, args.backend, args.mode)
  else:
    buildDir = os.path.join(baseDir, 'build', args.compiler, args.mode)
    installDir = os.path.join(baseDir, 'install', args.compiler, args.mode)
    
  if hasattr(args, 'float'):
    if args.float:
      REAL_TYPE = 'float'
    else:
      REAL_TYPE = 'double'

    buildDir = os.path.join(buildDir, REAL_TYPE)
    installDir = os.path.join(installDir, REAL_TYPE)
    
  if not os.path.exists(srcDir):
    raise Exception("source directory " + srcDir + " doesn't exist")
  if not os.path.exists(buildDir):
    os.makedirs(buildDir)

  cmake_params = ['-DCMAKE_INSTALL_MESSAGE=LAZY']
  cmake_params.append(args.cmake_gen)
  cmake_params.append('-DCMAKE_INSTALL_PREFIX=' + installDir)
  cmake_params.append('-DCMAKE_BUILD_TYPE=' + args.mode.capitalize())
  if hasattr(args, 'float') and args.float:
      cmake_params.append('-DFLOAT_TYPE=1')
  cmake_params += args.cmake_params
  
  commands = [
    ['cmake'] + cmake_params + ['-S', srcDir, '-B', buildDir],
    ['cmake', '--build', buildDir],
    ['cmake', '--install', buildDir]
  ]

  for c in commands:
    printCommand(c, args.baseDir)
    err = subprocess.call(c, env=args.env)
    if not err == 0:
      raise  Exception("Erreur")

def build_kokkos(args):
  
  class A:
    pass
  a = A()
  for p in args.__dict__.keys():
    if not p == 'float':
       exec("a." + p + '= args.' + p)
  
  kokkosBaseDir = os.path.join(a.baseDir, 'kokkos')
  if not os.path.exists(os.path.join(kokkosBaseDir, 'src')):
    os.makedirs(kokkosBaseDir, exist_ok=True)
    subprocess.run(['git', 'clone', 'git@github.com:kokkos/kokkos.git'],
                   cwd=kokkosBaseDir)
    subprocess.run(['git', 'checkout', '5.0.1'],
                   cwd=os.path.join(kokkosBaseDir, 'kokkos'))
    os.rename(os.path.join(kokkosBaseDir, 'kokkos'),
              os.path.join(kokkosBaseDir, 'src'))
    shutil.rmtree(os.path.join(kokkosBaseDir, 'src', '.git'), ignore_errors=True)

  for b in ['serial', 'openmp', 'cuda']:
    a.backend = b
    a.cmake_params = ['-DKokkos_ENABLE_SERIAL=ON']
    if b == 'openmp':
      a.cmake_params.append('-DKokkos_ENABLE_OPENMP=ON')
    elif b == 'cuda':
      a.cmake_params.append('-DKokkos_ENABLE_CUDA=ON')
      
    build(a, 'kokkos')
    
def build_code(args):
  class A:
    pass
  a = A()
  for p in args.__dict__.keys():
    exec("a." + p + '= args.' + p)

  for b in ['serial', 'openmp', 'cuda']:
    a.backend = b
    a.cmake_params = ['-DKokkos_DIR=' +
                           os.path.join(a.baseDir, 'kokkos', 'install',
                                        a.compiler, a.backend, a.mode, 
                                        'lib', 'cmake', 'Kokkos'),
                          '-DVERSION=' + b]
    build(a, None)
 
if __name__ == '__main__':

  args = init()
  build_kokkos(args)
  build_code(args)


