#!/usr/bin/env python
#
# Reads and prints the version info from indra/newview/viewerinfo.cpp.
#

def default_version_file():
    import os, sys

    scripts_dir = sys.path[0] # directory containing this script
    viewerinfo = os.path.join('indra', 'newview', 'viewerinfo.cpp')
    filepath = os.path.join(scripts_dir, '..', viewerinfo)

    return os.path.abspath(filepath)


def get_version(filepath):
    f = open(filepath)
    data = f.read()
    f.close()

    import re

    vals = {
        'major':  re.search('MAJOR\s*=\s*(\d+)',     data).group(1),
        'minor':  re.search('MINOR\s*=\s*(\d+)',     data).group(1),
        'patch':  re.search('PATCH\s*=\s*(\d+)',     data).group(1),
        'extra':  re.search('EXTRA\s*=\s*"([^"]*)"', data).group(1)
    }

    version = "%(major)s.%(minor)s.%(patch)s" % vals

    if len(vals['extra']) > 0:
        import string
        # Replace spaces and some puncuation with '-' in extra
        vals['extra'] = re.sub('[- \t:;,!+/\\"\'`]+', '-', vals['extra'])
        # Strip any leading or trailing "-"s
        vals['extra'] = string.strip(vals['extra'], '-')
        version += "-%(extra)s" % vals

    return version


if __name__ == '__main__':
    import errno, sys

    try:
        filepath = sys.argv[1]
    except IndexError:
        filepath = default_version_file()

    try:
        print get_version(filepath)
    except IOError, err:
        if err.errno == errno.ENOENT:
            print >> sys.stderr, 'File not found:', filepath
            sys.exit(1)
        else:
            raise
    except AttributeError:
        print >> sys.stderr, 'Error: malformatted file: ', filepath
        sys.exit(1)
