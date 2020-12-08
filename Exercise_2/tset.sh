#!/bin/bash

VAR=$(printf '.%.s' {1..6143})
for((i = 0;i<682;i+=1))
do
  echo $VAR > /dev/opsysmem
done


for((j=0;j<682;j+=1))
do
 cat /dev/opsysmem
done
