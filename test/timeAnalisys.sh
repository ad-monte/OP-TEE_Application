#!/bin/sh
# Compare timing for correct vs incorrect password when calling ta_secret -e

FILE="test1.txt"
KEY=23
ATTEMPTS=10
CORRECT="Alfonso"
WRONG="alsx"   # same length as CORRECT

# Pre-warm to reduce first-run noise
ta_secret -e "$FILE" "$KEY" "warmup" >/dev/null 2>&1 || true

# Monotonic milliseconds (prefer /proc/uptime; fallback to date)
now_ms() {
  if [ -r /proc/uptime ]; then
    # integer ms from uptime (monotonic)
    awk '{printf "%d\n", $1*1000}' /proc/uptime
    return
  fi
  ms="$(date +%s%3N 2>/dev/null)" || ms=""
  case "$ms" in
    (''|*[!0-9]*) s="$(date +%s)"; ms=$((s*1000)) ;;
  esac
  printf "%s\n" "$ms"
}

measure_avg_ms() {
  pwd="$1"
  total=0
  i=1
  while [ "$i" -le "$ATTEMPTS" ]; do
    start="$(now_ms)"
    ta_secret -e "$FILE" "$KEY" "$pwd" >/dev/null 2>&1 || true
    end="$(now_ms)"
    dur=$((end - start))
    total=$((total + dur))
    i=$((i + 1))
  done
  printf "%s\n" $((total / ATTEMPTS))
}

correct_ms="$(measure_avg_ms "$CORRECT")"
wrong_ms="$(measure_avg_ms "$WRONG")"
delta=$((correct_ms - wrong_ms))

printf "Attempts: %d\n" "$ATTEMPTS"
printf "Correct (%s):  %d ms (avg)\n" "$CORRECT" "$correct_ms"
printf "Wrong   (%s):  %d ms (avg)\n" "$WRONG"   "$wrong_ms"
printf "Delta (correct - wrong): %d ms\n" "$delta"