#!/bin/sh
if [ -f build.tag ];then echo Last build is No. `cat build.tag` ; else (echo 0 > build.tag); echo This is the first build.;fi
expr `cat build.tag` + 1 >build.tag
echo "#define _BUILD_ `cat build.tag`" > build.h
