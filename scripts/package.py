#!/usr/bin/env python
#
# @file package.py
# @author Jacek Antonelli
# @brief Script for generating viewer installer packages.
#
# Usage:  package.py --build-dir=PATH [options]
#
# Copyright (c) 2007-2009, Linden Research, Inc.
# Copyright (c) 2010-2011, Jacek Antonelli
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

import os, sys
from viewer_info import ViewerInfo


SCRIPTS_DIR = sys.path[0] # directory containing this script
TOP_DIR = os.path.abspath(os.path.join(SCRIPTS_DIR,'..'))
SOURCE_DIR = os.path.abspath(os.path.join(TOP_DIR,'indra'))
BUILD_TYPE = "RelWithDebInfo"


class PackagerError(Exception): pass

class BadDir(PackagerError): pass

class WeirdPlatform(PackagerError): pass

class CmdFailed(PackagerError): pass


def indent(text, amount=4):
    import string
    lines = [(' '*amount + line) for line in string.split(text, '\n')]
    return string.join(lines, '\n')

def message(*args):
    """Prints an informational message with a leading '#'."""
    print '# ' + ' '.join([str(arg) for arg in args])

def error(*args):
    """Prints an error message to stderr."""
    print >> sys.stderr, 'Error: ' + ' '.join([str(arg) for arg in args])


class Packager:

    def __init__(self, build_dir, opts={}):
        options = {'dest_dir': TOP_DIR,
                   'source_dir': SOURCE_DIR,
                   'build_type': BUILD_TYPE,
                   'verbose': False,
                   }
        options.update(opts)

        self.build_dir = os.path.abspath(build_dir)
        self.__check_build_dir()

        self.dest_dir = os.path.abspath(options['dest_dir'])
        self.__check_dest_dir()

        self.source_dir = os.path.abspath(options['source_dir'])
        self.__check_source_dir()

        self.build_type = options['build_type']
        self.platform = self.__get_platform()
        self.verbose = options['verbose']
        self.viewer_info = ViewerInfo()


    def make(self):
        plat = self.platform
        if   plat == 'linux':    self.make_linux()
        elif plat == 'mac':      self.make_mac()
        elif plat == 'windows':  self.make_windows()


    #########
    # LINUX #
    #########

    def make_linux(self):
        packaged_dir = os.path.join(self.build_dir, 'newview', 'packaged')

        if not os.path.exists(packaged_dir):
            raise BadDir("invalid build dir, has no 'newview/packaged/' "
                         'subdirectory: %(d)r'%{'d': self.build_dir})

        self.__run_command( 'Checking/fixing file permissions...',
"""find %(d)r -type d | xargs --no-run-if-empty chmod 755;
find %(d)r -type f -perm 0700 | xargs --no-run-if-empty chmod 0755;
find %(d)r -type f -perm 0500 | xargs --no-run-if-empty chmod 0555;
find %(d)r -type f -perm 0600 | xargs --no-run-if-empty chmod 0644;
find %(d)r -type f -perm 0400 | xargs --no-run-if-empty chmod 0444;
true""" % {'d': packaged_dir})

        plat = 'Linux'
        from platform import architecture
        if architecture()[0] == '64bit':
            plat += '64'
        elif architecture()[0] == '32bit':
            plat += '32'

        inst_name = self.viewer_info.combined + '-' + plat
        dest_file = os.path.join(self.dest_dir, inst_name + '.tar.bz2')

        self.__run_command('Creating package %r...'%dest_file,
            'tar -C %(dir)s --numeric-owner '
            '--transform "s,^./,%(inst_name)s/," '
            #'--verbose --show-transformed-names '
            '-cjf %(dest_file)s .' % { 'dir': packaged_dir,
                                       'inst_name': inst_name,
                                       'dest_file': dest_file})

        message('Package complete: %r' % dest_file)


    #######
    # MAC #
    #######

    def make_mac(self):
        import shutil

        # Where the DMG files (.DS_Store, background image, etc.) come from.
        dmg_src = os.path.join(self.source_dir, 'newview', 'packaging', 'mac', 'dmg')

        # Staging directory. Everything that will be in the package is
        # copied to this dir, then a DMG is created from it.
        dmg_dst = os.path.join(self.build_dir, 'pack')

        if (os.path.exists(dmg_dst)):
            message("Removing stale staging dir %r..." % dmg_dst)
            shutil.rmtree(dmg_dst)

        message("Creating staging dir %r..." % dmg_dst)
        os.makedirs(dmg_dst)

        for src,dst in {"_VolumeIcon.icns": ".VolumeIcon.icns",
                        "background.jpg":   "background.jpg",
                        "_DS_Store":        ".DS_Store",
                        }.items():

            message("Copying %r..." % dst)
            shutil.copy2( os.path.join(dmg_src, src),
                          os.path.join(dmg_dst, dst))

            self.__run_command(
                'Hiding %r...' % dst,
                'SetFile -a V %r' % os.path.join(dmg_dst, dst))

        
        # Create the alias file (which is a resource file) from the
        # .r file, then clean up the .r file.
        self.__run_command(
            'Creating Applications alias...',
            'Rez %r -o %r' % (os.path.join(dmg_src, "Applications-alias.r"),
                              os.path.join(dmg_dst, "Applications")))

        self.__run_command(
            "Setting the Applications alias's alias and custom icon bits...",
            'SetFile -a AC %r' % os.path.join(dmg_dst, "Applications"))

        self.__run_command(
            "Setting disk image root's custom icon bit...",
            'SetFile -a C %r' % dmg_dst)

        app_name = self.viewer_info.name + ".app"
        app_orig = os.path.join(self.build_dir, 'newview', self.build_type, app_name)
        app_dst = os.path.join(dmg_dst, app_name)

        if (not os.path.exists(app_orig)):
            error("App does not exist: %r" % app_orig)
            sys.exit(1)

        # Move the .app to the staging area (temporarily).
        message("Copying %r (this takes a while)..."%(app_name))
        shutil.copytree(app_orig, app_dst, symlinks=True)

        dmg_name = "%s-Mac"%(self.viewer_info.combined)
        temp_dmg = os.path.join(self.build_dir, dmg_name+".temp.dmg")
        final_dmg = os.path.join(self.dest_dir, dmg_name+".dmg")

        if (os.path.exists(temp_dmg)):
            message("Removing stale temp disk image...")
            os.remove(temp_dmg)

        # MBW -- If the mounted volume name changes, it breaks the
        # .DS_Store's background image and icon positioning. If we
        # really need differently named volumes, we'll need to create
        # multiple DS_Store file images, or use some other trick.
        volname="Second Life Installer"  # DO NOT CHANGE without understanding comment above

        self.__run_command(
            'Creating temp disk image (this takes a while)...',
            'hdiutil create %(temp)r -volname %(vol)r -fs HFS+ '
            '-layout SPUD -srcfolder %(src)s' %
            {'temp': temp_dmg, 'vol': volname, 'src': dmg_dst})

        if (os.path.exists(final_dmg)):
            bkp = final_dmg + ".bkp"
            message("Renaming existing final disk image to %r..." % bkp)
            shutil.move(final_dmg, bkp)

        self.__run_command(
            'Creating compressed final disk image (this takes a while)...',
            'hdiutil convert %(temp)r -format UDBZ -o %(final)r' %
            {'temp':temp_dmg, 'final':final_dmg})

        message("Removing temp disk image...")
        os.remove(temp_dmg)

        message("Removing staging dir...")
        shutil.rmtree(dmg_dst)

        message('Package complete: %r'%final_dmg)


    ###########
    # WINDOWS #
    ###########

    def make_windows(self):
        print "Packaging for Windows is not supported yet."


    ###################
    # PRIVATE METHODS #
    ###################

    def __check_build_dir(self):
        if not os.path.exists(self.build_dir):
            raise BadDir('build dir %(dir)r does not exist.' %
                         {'dir': self.build_dir})
        if not os.path.isdir(self.build_dir):
            raise BadDir('build dir %(dir)r is not a directory.' %
                         {'dir': self.build_dir})

    def __check_dest_dir(self):
        if not os.path.exists(self.dest_dir):
            raise BadDir('dest dir %(dir)r does not exist.' %
                         {'dir': self.dest_dir})
        if not os.path.isdir(self.dest_dir):
            raise BadDir('dest dir %(dir)r is not a directory.' %
                         {'dir': self.dest_dir})
        if not os.access(self.dest_dir, os.W_OK):
            raise BadDir('dest dir %(dir)r is not writable.' %
                         {'dir': self.dest_dir})

    def __check_source_dir(self):
        if not os.path.exists(self.source_dir):
            raise BadDir('source dir %(dir)r does not exist.' %
                         {'dir': self.source_dir})
        if not os.path.isdir(self.source_dir):
            raise BadDir('source dir %(dir)r is not a directory.' %
                         {'dir': self.source_dir})

    def __get_platform(self):
        platform = sys.platform
        try:
            return {'linux2':'linux',
                    'linux1':'linux',
                    'cygwin':'windows',
                    'win32' :'windows',
                    'darwin':'mac',
                    }[platform]
        except KeyError:
            raise WeirdPlatform(
                "Unrecognized platform/operating system: %r" % platform)

    def __run_command(self, summary=None, command=None):
        if summary: message(summary)

        if not command: return

        import subprocess

        out = subprocess.PIPE # = intercept command's output
        if self.verbose:
            print indent(command)
            out = None # = don't intercept

        child = subprocess.Popen(command, shell=True, stdout=out, stderr=None)
        status = child.wait()

        if status:
            raise CmdFailed('A command returned non-zero status (%s):\n%s'%
                            (status, indent(command)))



def main(args=sys.argv[1:]):
    from optparse import OptionParser

    op = OptionParser(usage='%prog --build-dir=PATH [options]')

    op.add_option('--build-dir', dest='build_dir', nargs=1, metavar='PATH',
                  help='path to the \'build\' directory, which contains '
                  'CMakeCache.txt and the compile result subdirectories')

    op.add_option('--dest-dir', dest='dest_dir', nargs=1, metavar='PATH',
                  default=TOP_DIR,
                  help='optional path to destination directory where the '
                  'package result should be saved. Default: %(TOP_DIR)r'
                  %{ 'TOP_DIR': TOP_DIR } )

    op.add_option('--source-dir', dest='source_dir', nargs=1, metavar='PATH',
                  default=SOURCE_DIR,
                  help='optional path to an alternate source directory, '
                  'i.e. \'indra\'. Default: %(SOURCE_DIR)r'
                  %{ 'SOURCE_DIR': SOURCE_DIR } )

    op.add_option('--build-type', dest='build_type', nargs=1, metavar='TYPE',
                  default=BUILD_TYPE,
                  help='\'Debug\', \'RelWithDebInfo\', or \'Release\'. '
                  'Default: %(BUILD_TYPE)r'
                  %{ 'BUILD_TYPE': BUILD_TYPE } )

    op.add_option('-v', '--verbose', action='store_true', default=False,
                  help='print all shell commands as they are run')

    if not args:
        op.print_help()
        return

    options = op.parse_args(args)[0]

    if not options.build_dir:
        error('--build-dir=PATH is required.')
        sys.exit(1)

    opts_dict = {'dest_dir': options.dest_dir,
                 'source_dir': options.source_dir,
                 'build_type': options.build_type,
                 'verbose':  options.verbose}

    try:
        Packager(options.build_dir, opts_dict).make()
    except PackagerError, err:
        error(err.args[0])
        sys.exit(1)


if __name__ == '__main__':
    main()
