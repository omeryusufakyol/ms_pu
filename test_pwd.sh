#!/bin/bash

# LOG klasörü oluştur
mkdir -p logs

# Geçici klasörler
mkdir -p test_pwd_dir/subdir

# Test komutları
tests=(
  "pwd"
  "cd test_pwd_dir"
  "pwd"
  "cd subdir"
  "pwd"
  "cd ../.."
  "pwd"
  "cd not_a_real_dir" # should error
  "pwd"
)

i=1
for cmd in "${tests[@]}"
do
  printf "Running test %02d: %s\n" "$i" "$cmd"

  # Bash output
  bash -c "$cmd" > logs/bash_out.txt 2>&1

  # minishell output
  echo "$cmd" > logs/cmd.txt
  echo "exit" >> logs/cmd.txt
  ./minishell < logs/cmd.txt > logs/mini_out.txt 2>&1

  # "exit" satırını temizle
  sed -i '' '/^exit$/d' logs/mini_out.txt 2>/dev/null || sed -i '/^exit$/d' logs/mini_out.txt

  # Karşılaştır
  if diff -u logs/bash_out.txt logs/mini_out.txt > logs/pwd_fail_$i.diff ; then
    echo "✅ PASS"
    rm -f logs/pwd_fail_$i.diff
  else
    echo "❌ FAIL (see logs/pwd_fail_$i.diff)"
  fi
  ((i++))
done

# Temizlik
rm -f logs/bash_out.txt logs/mini_out.txt logs/cmd.txt
rm -rf test_pwd_dir
