#!/bin/sh

#Author: Alexander Lutsai <s.lyra@ya.ru>



source_name="gui_bitmaps.c"
header_name="gui_bitmaps.h"

#script's directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
#root of the project
root="$DIR/../.."
#path to generator script
util="$root/osfi-z/MakiseGUI/utils/bitmap_creator/img_to_code.py"
#output paths
out_c="$root/osfi-z/gui/$source_name"
out_h="$root/osfi-z/gui/$header_name"
#generator command
command="python3 $util $DIR/"

function print_bitmaps {
    #HERE ADD YOUR BITMAPS. LIKE
    #
    ${command}folder9x8.png folder $1
    ${command}folder9x8.png fucky $1
}

echo "Generating header ${header_name}..."

echo "#ifndef GUI_BITMAPS_H" > $out_h
echo "#define GUI_BITMAPS_H" >> $out_h
echo "#include \"makise_gui.h\"" >> $out_h
echo "" >> $out_h

print_bitmaps -d >> $out_h

echo "#endif" >> $out_h

echo "Header $out_h generated"
echo "Generating source ${source_name}..."

echo "#include \"$header_name\"" > $out_c
echo "" >> $out_c

print_bitmaps >> $out_c

echo "Source $out_c generated"
echo "DONE"
