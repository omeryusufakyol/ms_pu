#!/bin/bash

mkdir -p logs
mkdir -p tmp

tests=(
  "echo Hello > tmp/out1.txt"
  "cat tmp/out1.txt"
  "echo Another Line >> tmp/out1.txt"
  "cat tmp/out1.txt"
  "echo Overwrite > tmp/out1.txt"
  "cat tmp/out1.txt"
  "echo Test > tmp/out2.txt"
  "cat < tmp/out2.txt"
  "cat < tmp/out1.txt"
  "echo Hello > tmp/file with spaces.txt"
  "cat tmp/file with spaces.txt"
  "echo Hidden > tmp/.hiddenfile"
  "cat tmp/.hiddenfile"
  "cat < tmp/nonexistent.txt"
)

i=1
for cmd in "${tests[@]}"
do
  printf "Running test %02d: %s\n" "$i" "$cmd"

  # Bash
  bash -c "$cmd" > logs/bash_out.txt 2>&1

  # Minishell
  echo "$cmd" > logs/cmd.txt
  echo "exit" >> logs/cmd.txt
  ./minishell < logs/cmd.txt > logs/mini_out.txt 2>&1

  # Remove "exit" line
  sed -i '' '/^exit$/d' logs/mini_out.txt 2>/dev/null || sed -i '/^exit$/d' logs/mini_out.txt

  # Diff
  if diff -u logs/bash_out.txt logs/mini_out.txt > logs/redirect_fail_$i.diff ; then
    echo "✅ PASS"
    rm -f logs/redirect_fail_$i.diff
  else
    echo "❌ FAIL (see logs/redirect_fail_$i.diff)"
  fi
  ((i++))
done

rm -f logs/bash_out.txt logs/mini_out.txt logs/cmd.txt
