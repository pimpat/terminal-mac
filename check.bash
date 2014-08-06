#!/bin/bash
if [ -f "ps.out" ];
then
 echo "Correct file in place"
else
 echo "No such file!!!"
fi
sed 's/..............................................................................................$//g' ps.out | sed 's/^........//g' > ps-sed.out
cut -c 9-11 ps.out > ps-cut.out
cat ps.out | awk '{print $2}' > ps-awk.out 
echo "successful"
