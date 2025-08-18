#!/bin/bash

# LOG klasörü oluştur
mkdir -p logs

# Test komutları
tests=(
  "echo hello world"
  "echo \"hello world\""
  "echo 'hello world'"
  "export VAR1=hello"
  "echo \"\$VAR1\""
  "echo \"\$VAR1 world\""
  "echo \"\$VAR1\$VAR1\""
  "echo '\$VAR1'"
  "echo '\$VAR1\$VAR1'"
  "echo \$?"
  "echo \"\$?\""
  "echo \"\$?\"abc"
  "echo abc\"\$?\""
  "echo \"\$?\"abc\"\$?\"abc"
  "echo \$"
  "echo \"\$\""
  "echo '\$'"
  "echo \$NOT_DEFINED_VAR"
  "echo \"\$NOT_DEFINED_VAR\""
  "echo '\$NOT_DEFINED_VAR'"
  "echo ~"
  "echo ~/"
  "echo ~/folder"
  "echo \"~/folder\""
  "echo '~/folder'"
  "echo \"\""
  "echo ''"
  "echo \"\$VAR1\$NOT_DEFINED_VAR\""
  "echo \"'\$VAR1'\""
  "echo '\"\$VAR1\"'"
  "echo \"hello '\$VAR1'\""
  "echo \"hello \\\"\$VAR1\\\"\""
  "echo 'hello \"\$VAR1\"'"
  "echo a    b     c"
  "echo     a    b    c"
  "echo 'a    b     c'"
  "echo \"a    b     c\""
  "echo -n test"
  "echo -n \"\$VAR1\""
)

i=1
for cmd in "${tests[@]}"
do
  printf "Running test %02d: %s\n" "$i" "$cmd"

  # BASH output
  bash -c "$cmd" > logs/bash_out.txt 2>&1

  # minishell output (komutu dosyadan al)
  echo "$cmd" > logs/cmd.txt
  echo "exit" >> logs/cmd.txt
  ./minishell < logs/cmd.txt > logs/mini_out.txt 2>&1

  # minishell son satırı "exit" mi, sil
  sed -i '' '/^exit$/d' logs/mini_out.txt 2>/dev/null || sed -i '/^exit$/d' logs/mini_out.txt

  # Compare
  if diff -u logs/bash_out.txt logs/mini_out.txt > logs/echo_fail_$i.diff ; then
    echo "✅ PASS"
    rm -f logs/echo_fail_$i.diff
  else
    echo "❌ FAIL (see logs/echo_fail_$i.diff)"
  fi
  ((i++))
done

# Temizlik
rm -f logs/bash_out.txt logs/mini_out.txt logs/cmd.txt
