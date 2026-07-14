#!/bin/sh
# To prevent the guest from deadlocking, we must execute the rescue command immediately 
# after the freeze command, before the guest kernel has time to attempt a TLB flush or trigger the watchdog.
echo "Freezing Core 0..."
taskset 1 cat /proc/thesis_target &
sleep 1
echo "Rescuing with Core 1..."
taskset 2 cat /proc/thesis_rescue
