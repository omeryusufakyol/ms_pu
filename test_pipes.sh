#!/bin/bash

mkdir -p logs

tests=(
  "echo hello | cat"
  "echo hello | wc -c"
  "ls | wc -l"
  "echo hello | grep h"
  "echo hello | grep x"
  "cat /etc/passwd | grep root"
  "echo line1 && echo line2 | wc -l"  # bu geçersiz, testte yok
  "echo test | cat | cat | cat"
  "echo test |"
  "| echo test"
  "cat < nonexistent.txt | grep hello"
  "echo -n hello | wc -c"
  "printf \"a\\nb\\nc\\n\" | grep b"
  "printf \"a\\nb\\nc\\n\" | grep x"
  "echo 123 | tr 0-9 a-j"
)

i=1
for cmd in "${tests[@]}"
do
  printf "Running test %02d: %s\n" "$i" "$cmd"

  bash -c "$cmd" > logs/bash_out.txt 2>&1

  echo "$cmd" > logs/cmd.txt
  echo "exit" >> logs/cmd.txt
  ./minishell < logs/cmd.txt > logs/mini_out.txt 2>&1

  sed -i '' '/^exit$/d' logs/mini_out.txt 2>/dev/null || sed -i '/^exit$/d' logs/mini_out.txt

  if diff -u logs/bash_out.txt logs/mini_out.txt > logs/pipe_fail_$i.diff ; then
    echo "✅ PASS"
    rm -f logs/pipe_fail_$i.diff
  else
    echo "❌ FAIL (see logs/pipe_fail_$i.diff)"
  fi
  ((i++))
done

rm -f logs/bash_out.txt logs/mini_out.txt logs/cmd.txt
