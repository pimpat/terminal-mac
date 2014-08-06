#!/bin/tcsh
if ( -e "ps.out" ) then
 echo "Collect file in place"
else
 echo "No such file!!!"
endif
sed 's/..............................................................................................$//g' ps.out | sed 's/^........//g' > ps-sed.out
cut -c 9-11 ps.out > ps-cut.out
cat ps.out | awk '{print $2}' > ps-awk.out
echo "successful"
