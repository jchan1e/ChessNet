#!/bin/bash

echo $2 > $1.agent
echo $3 >> $1.agent
echo $4 >> $1.agent
echo $1.nn >> $1.agent

./createNN $1.nn ${@:5}
