#!/usr/bin/env python3
"""
  Copyright (c) 2014 Ivor Wanders

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
"""

"""
  This allows easy creation of *uino's keywords.txt file from the source files.
"""

import os
import re

prefix = """#######################################
# Syntax Coloring for {project_name}
#######################################

#######################################
# Datatypes (KEYWORD1)
#######################################
{datatypes}
#######################################
# Instances (KEYWORD2)
#######################################
{instances}
#######################################
# Methods and Functions (KEYWORD2)
#######################################
{methods}
#######################################
# Constants (LITERAL1)
#######################################
{literals}
"""


def get_methods(file):
    matcher = re.compile("\s+[\w]+ +([^(-]+)\(")
    methods = []
    for line in file:
        a = matcher.findall(line)
        if (len(a) != 0) and (" " not in a[0]) and (":" not in a[0]):
            methods.append(a[0])
    return methods


def get_constants(file):
    matcher = re.compile("#define\s+([\w_]+)\s+")
    constants = []
    for line in file:
        a = matcher.findall(line)
        if (len(a) != 0) and (" " not in a[0]) and (":" not in a[0]):
            constants.append(a[0])
    return constants

if __name__ == "__main__":

    method_files = ["bareRFM69.h", "plainRFM69.h"]
    literal_files = ["bareRFM69_const.h"]
    data_types = ["bareRFM69", "plainRFM69"]
    instances = ["rfm"]
    project_name = "plainRFM69"
    file_path = ".."

    methods = []
    for filename in method_files:
        with open(os.path.join(file_path, filename)) as f:
            methods.extend(get_methods(f))

    for filename in literal_files:
        with open(os.path.join(file_path, filename)) as f:
            constants = get_constants(f)

    foo = ["{:} LITERAL1".format(c) for c in constants]
    literal_string = "\n".join(["{}\tLITERAL1".format(c) for c in constants])
    method_string = "\n".join(["{}\tKEYWORD2".format(c) for c in methods])
    data_t_string = "\n".join(["{}\tKEYWORD1".format(c) for c in data_types])
    instance_string = "\n".join(["{}\tKEYWORD2".format(c) for c in instances])
    done = prefix.format(literals=literal_string, methods=method_string,
                         project_name=project_name, datatypes=data_t_string,
                         instances=instance_string)
    print(done)
