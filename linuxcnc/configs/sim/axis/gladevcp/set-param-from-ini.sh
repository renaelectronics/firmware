#!/bin/bash

cd $(dirname $::argv0)

gladevcp -u ./meter_scale.py ./meter_scale.ui &

# wait for gladevcp startup
sleep 3
# start sim_pin to exercise pins
sim_pin \
   meter_scale.hal_table1 \
   meter_scale.max-value \
   meter_scale.meter \

exit 0
