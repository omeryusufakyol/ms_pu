#!/bin/bash

# LOG klasörü oluştur
mkdir -p logs

# Geçici klasörler oluştur
mkdir -p test_dir1 test_dir2

# Test komutları
tests=(
  "pwd"
  "cd test_dir1"
  "pwd"
  "cd .."
  "pwd"
  "cd ./test_dir2"
  "pwd"
  "cd ../"
  "pwd"
  "cd"
  "pwd"
  "cd -"
  "pwd"
  "cd not_exist_dir"
  "cd .. && pwd" # bu çalışmamalı çünkü && desteklenmiyor
)

i=1
for cmd in "${tests[@]}"
do
  printf "Running test %02d: %s\n" "$i" "$cmd"

  # BASH output
  bash -c "$cmd" > logs/bash_out.txt 2>&1

  # minishell output
  echo "$cmd" > logs/cmd.txt
  echo "exit" >> logs/cmd.txt
  ./minishell < logs/cmd.txt > logs/mini_out.txt 2>&1

  # minishell son satırı "exit" mi, sil
  sed -i '' '/^exit$/d' logs/mini_out.txt 2>/dev/null || sed -i '/^exit$/d' logs/mini_out.txt

  # Compare
  if diff -u logs/bash_out.txt logs/mini_out.txt > logs/cd_fail_$i.diff ; then
    echo "✅ PASS"
    rm -f logs/cd_fail_$i.diff
  else
    echo "❌ FAIL (see logs/cd_fail_$i.diff)"
  fi
  ((i++))
done

# Temizlik
rm -f logs/bash_out.txt logs/mini_out.txt logs/cmd.txt
rm -rf test_dir1 test_dir2
