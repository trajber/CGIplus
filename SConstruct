# CGIplus Copyright (C) 2011 Rafael Dantas Justo
#
# This file is part of CGIplus.
#
# CGIplus is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CGIplus is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CGIplus.  If not, see <http://www.gnu.org/licenses/>.

import os

# Environment definitions

includePath = ["#include"]
libraryPath = ["#lib"]

myMode = ARGUMENTS.get("mode", "debug")
if myMode not in ['debug', 'release']:
    print "Error: expected 'debug' or 'release', found: " + myMode
    Exit(1)

compilerFlags = ["-pipe", "-Wall", "-Werror", "-std=c++0x"]
if "debug" in myMode:
    compilerFlags.append("-g")
elif "release" in myMode:
    compilerFlags.append("-O2")

cflags = ARGUMENTS.get("cflags", "")
for cflagsItem in cflags.split(" "):
    compilerFlags.append(cflagsItem)

if "CXX" not in os.environ:
    os.environ["CXX"] = "g++"

env = Environment(AR = "ar", 
                  ARFLAGS = "-csrv", 
                  CXX = os.environ["CXX"], 
                  CPPPATH = includePath, 
                  CXXFLAGS = compilerFlags)

# Colorize

verbose = ARGUMENTS.get("verbose", "0")
if (verbose == "0"):
    colors = {}
    colors["cyan"]   = "\033[96m"
    colors["purple"] = "\033[95m"
    colors["blue"]   = "\033[94m"
    colors["green"]  = "\033[92m"
    colors["yellow"] = "\033[93m"
    colors["red"]    = "\033[91m"
    colors["end"]    = "\033[0m"

    cppSourceColor = "%s\nCompiling %s==> %s$SOURCE%s" % \
        (colors["blue"], colors["purple"], colors["yellow"], colors["end"])

    sharedCppColor = "%s\nCompiling shared %s==> %s$SOURCE%s" % \
        (colors["blue"], colors["purple"], colors["yellow"], colors["end"])

    linkProgramColor = "%s\nLinking Program %s==> %s$TARGET%s" % \
        (colors["red"], colors["purple"], colors["yellow"], colors["end"])

    staticLibraryColor = "%s\nLinking Static Library %s==> %s$TARGET%s" % \
        (colors["red"], colors["purple"], colors["yellow"], colors["end"])

    ranlibColor = "%s\nRanlib Library %s==> %s$TARGET%s" % \
        (colors["red"], colors["purple"], colors["yellow"], colors["end"])

    sharedLibraryColor = "%s\nLinking Shared Library %s==> %s$TARGET%s" % \
        (colors["red"], colors["purple"], colors["yellow"], colors["end"])

    env["CXXCOMSTR"] = cppSourceColor
    env["CCCOMSTR"] = cppSourceColor
    env["SHCCCOMSTR"] = sharedCppColor
    env["SHCXXCOMSTR"] = sharedCppColor
    env["ARCOMSTR"] = staticLibraryColor
    env["RANLIBCOMSTR"] = ranlibColor
    env["SHLINKCOMSTR"] = sharedLibraryColor
    env["LINKCOMSTR"] = linkProgramColor

# Libraries

libraries = {
    "CGIPLUS" : ["cgiplus", "boost_system", "boost_regex"]
    }

def getLibraries(names):
    localLibraries = []
    for name in names:
        try:
            localLibraries.extend(libraries[name])
        except KeyError:
            print "Librarie " + name + " not defined"

    return localLibraries

# Compile destinations

basePath = os.getcwd()
localLibraryInstall = "#lib"
localBinInstall = "#bin"

env.Clean(".", localLibraryInstall)
env.Clean(".", localBinInstall)

# Test
# http://www.scons.org/wiki/UnitTests
# ------------------------------------------------------
def builder_unit_test(target, source, env):
    app = str(source[0].abspath)
    os.spawnl(os.P_WAIT, app, app, "--show_progress=yes")

# Create a builder for tests
bld = Builder(action = builder_unit_test)
env.Append(BUILDERS = {'Test' :  bld})
# ------------------------------------------------------

# Documentation

def PhonyTargets(env = None, **kw):
    if not env: env = DefaultEnvironment()
    for target,action in kw.items():
        env.AlwaysBuild(env.Alias(target, [], action))

PhonyTargets(env, doc  = "cd doc; doxygen doxygen.cfg")

# Export for modules

Export("env",
       "basePath",
       "localLibraryInstall", 
       "localBinInstall",
       "libraryPath",
       "getLibraries")

# Find and run modules

def findSConscripts(dir):
    sconscripts = []
    for name in os.listdir(dir):
        fullpath = os.path.join(dir, name)
        if (".svn" in fullpath):
            continue

        if (os.path.isdir(fullpath) == False):
            if ("SConscript" in fullpath):
                sconscripts.append(fullpath)
        else:
            sconscripts.extend(findSConscripts(fullpath))

    return sconscripts

SConscript(findSConscripts("."))
