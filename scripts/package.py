#!/usr/bin/env python
#
# @file package.py
# @author Jacek Antonelli
# @brief Script for generating viewer installer packages.
#
# Usage:  package.py --build-dir=PATH [options]
#
# Copyright (c) 2007-2009, Linden Research, Inc.
# Copyright (c) 2010, Jacek Antonelli
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


class PackagerError(Exception): pass

class BadDir(PackagerError): pass

class WeirdPlatform(PackagerError): pass

class CmdFailed(PackagerError): pass


def indent(text, amount=4):
    import string
    lines = [(' '*amount + line) for line in string.split(text, '\n')]
    return string.join(lines, '\n')



class Packager:

    def __init__(self, build_dir, opts={}):
        options = {'dest_dir': TOP_DIR,
                   'source_dir': SOURCE_DIR,
                   'verbose': False,
                   }
        options.update(opts)

        self.build_dir = os.path.abspath(build_dir)
        self.__check_build_dir()

        self.dest_dir = os.path.abspath(options['dest_dir'])
        self.__check_dest_dir()

        self.source_dir = os.path.abspath(options['source_dir'])
        self.__check_source_dir()

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

        print '# Package complete: %r'%dest_file


    #######
    # MAC #
    #######

    def make_mac(self):
        print "Packaging for Mac OS X is not supported yet."


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
        if summary: print "# " + summary

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

    op.add_option('-v', '--verbose', action='store_true', default=False,
                  help='print all shell commands as they are run')

    if not args:
        op.print_help()
        return

    options = op.parse_args(args)[0]

    if not options.build_dir:
        print >> sys.stderr, 'Error: --build-dir=PATH is required.'
        sys.exit(1)

    opts_dict = {'dest_dir': options.dest_dir,
                 'source_dir': options.source_dir,
                 'verbose':  options.verbose}

    try:
        Packager(options.build_dir, opts_dict).make()
    except PackagerError, err:
        print >> sys.stderr, 'Error: %s' % err.args[0]
        sys.exit(1)


if __name__ == '__main__':
    main()
