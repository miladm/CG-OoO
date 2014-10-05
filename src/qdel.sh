#!/bin/bash
begin=$1
end=$2

for (( i=$begin; i<=$end; i++ ))
do
	qdel "$i"
done
