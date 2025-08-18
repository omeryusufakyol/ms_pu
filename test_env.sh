#!/bin/bash

mkdir -p logs

tests=(
  "env"
  "export TEST1=value1"
  "export TEST2=value2"
  "env"
  "unset TEST1"
  "env"
  "unset TEST2"
  "env"
)

i=1
for cmd in "${tests[@]}"
do
  printf "Running test %02d: %s\n" "$i" "$cmd"

  # Bash çıktısı
  bash -c "$cmd" > logs/bash_out.txt 2>&1

  # minishell çıktısı
  echo "$cmd" > logs/cmd.txt
  echo "exit" >> logs/cmd.txt
  ./minishell < logs/cmd.txt > logs/mini_out.txt 2>&1

  # "exit" satırını temizle
  sed -i '' '/^exit$/d' logs/mini_out.txt 2>/dev/null || sed -i '/^exit$/d' logs/mini_out.txt

  # Karşılaştır
  if diff -u logs/bash_out.txt logs/mini_out.txt > logs/env_fail_$i.diff ; then
    echo "✅ PASS"
    rm -f logs/env_fail_$i.diff
  else
    echo "❌ FAIL (see logs/env_fail_$i.diff)"
  fi
  ((i++))
done

rm -f logs/bash_out.txt logs/mini_out.txt logs/cmd.txt
