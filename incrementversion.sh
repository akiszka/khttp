#!/bin/bash
a=`cat ./VERSION | cut -c8-`
let "a++"
a=alpha/b$a
echo $a > VERSION
echo $a

