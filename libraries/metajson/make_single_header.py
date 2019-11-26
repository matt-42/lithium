#! /usr/bin/python3

import tempfile
import subprocess
import os,re,sys,shutil

from os.path import join

def process_file(f, processed, output):

    if f in processed:
        return
    processed.append(f)

    for line in open(f, "r"):
        m = re.match("^#include\s*<(iod/.*)>.*$", line)
        if m:
            process_file(join(install_dir, "include", m.group(1)), processed, output)
        elif re.match("#pragma once", line):
            pass
        else:
            output.append(line)

if __name__ == "__main__":

    if os.path.exists(sys.argv[1]):
        print(sys.argv[1], "already exists")
        exit(1)
    

    output_file=open(sys.argv[1], 'w')

    # create temp directory.
    tmp_dir=tempfile.mkdtemp()
    src_dir=join(tmp_dir, "src")
    build_dir=join(tmp_dir, "build")
    install_dir=join(tmp_dir, "install")

    os.mkdir(src_dir)
    os.mkdir(build_dir)
    os.mkdir(install_dir)

    processed=[]

    # git clone recursive iod
    subprocess.check_call(["git", "clone", "--recursive", "https://github.com/iodcpp/iod", src_dir])

    # cd build_dir
    os.chdir(build_dir)

    # Install
    subprocess.check_call(["cmake", src_dir, "-DCMAKE_INSTALL_PREFIX=" + install_dir, "-DCMAKE_CXX_COMPILER=g++"])
    subprocess.check_call(["make", "install", "-j4"])

    # Generate single file header.
    lines=[]
    process_file(join(install_dir, "include/iod/metajson/metajson.hh"), processed, lines)

    body=[]
    includes=[]
    for line in lines:
        m = re.match("^\s*#include.*$", line)
        if m:
            includes.append(line)
        else:
            body.append(line)

    output_file.write("// Author: Matthieu Garrigues matthieu.garrigues@gmail.com\n//\n")
    output_file.write("// Single header version the iod/metajson library.\n")
    output_file.write("// https://github.com/iodcpp/metajson\n")
    output_file.write("//\n")    
    output_file.write("// Note: This file is generated.\n\n")
    output_file.write("#pragma once\n\n")
    for l in set(includes):
        output_file.write(l)
    output_file.write("\n\n")
    for l in body:
        output_file.write(l)
    output_file.close()
    
    print("metajson single header", sys.argv[1], "generated.")

    # Cleanup
    shutil.rmtree(tmp_dir)
