#!/bin/bash

func(){
  echo "Hello$1" > /dev/opsysmem
}

for i in {1..10}
do 
 func $i &
done

wait 
cat /dev/opsysmem
