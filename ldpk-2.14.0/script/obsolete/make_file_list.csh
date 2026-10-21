#!/bin/csh

set basedir=$LDPK/script
cd $basedir

python make_file_list.py -path $basedir/.. -linkbase ../../.. -format html -charset utf-8 -order dirs -exclude_dir .svn tmp test obsolete > ../doc/misc/files.html

cat ../doc/misc/pre_lof.html ../doc/misc/files.html ../doc/misc/post_lof.html > ../doc/misc/list_of_files.html
