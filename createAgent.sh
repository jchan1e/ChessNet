#!/bin/bash

# Args:
# 1 path to agent
# 2 training alpha
# 3 alpha decay ratio
# 4 montecarlo exploration bias
# 5 mutation chance
# 6+ NN layer sizes

echo $2 > $1.agent
echo $3 >> $1.agent
echo $4 >> $1.agent
echo $5 >> $1.agent
echo $1.nn >> $1.agent
echo ${@:6} >> $1.agent

./createNN $1.nn ${@:6}
