# this script is sdv-internal.

import sys
import string

def expand(path,replacements):
	txt_out = ""

	fin_in = open(path,"r")
	for tline in fin_in:
		tline_out = tline
		for tag,ref in replacements:
			if tag in tline_out:
				content = expand(ref,replacements)
				tline_out = tline_out.replace(tag,content)
		txt_out += tline_out
	return txt_out

replacements = []
fin_xpnd = open(sys.argv[2],"r")
for line in fin_xpnd:
	tag,ref = line.split("---")
	tag = tag.strip()
	ref = ref.strip()
	replacements.append([tag,ref])

print(expand(sys.argv[1],replacements))

