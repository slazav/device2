#!/bin/sh -efu

# simple SPP interface

echo "#SPP1"
echo "Hi!"
stdbuf -o L echo "#OK"

while read x; do
  if [ "$x" = "wait"  ];
    then sleep 10;
  fi
  if [ "$x" = error ]; then
    stdbuf -o L echo "#Error: some error";
  else
    echo "Q: $x"
    stdbuf -o L echo "#OK"
  fi
done