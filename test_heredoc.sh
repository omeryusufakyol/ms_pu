#!/bin/bash

mkdir -p logs

tests=(
  "cat << END"
  "cat << EOF"
  "cat << stop"
  "cat << STOP | wc -l"
  "cat << word1 << word2"
  "cat << QUOTE"
  "cat << end_marker"
  "cat << 'NOEXPAND'"
  "cat << DOLLAR"
)

heredoc_contents=(
  "hello from heredoc\nEND"
  "line1\necho \$HOME\nEOF"
  "first\nsecond\nstop"
  "line1\nline2\nSTOP"
  "one\ntwo\nword2"
  "hello world\nQUOTE"
  "multi-line\nwith spaces\nend_marker"
  "should_not_expand: \$USER\nNOEXPAND"
  "exit code is \$?\nDOLLAR"
)

i=1
for cmd in "${tests[@]}"
do
  printf "Running test %02d: %s\n" "$i" "$cmd"

  # BASH
  {
    echo "$cmd"
    echo -e "${heredoc_contents[$((i-1))]}"
    echo "exit"
  } | bash > logs/bash_out.txt 2>&1

  # minishell
  {
    echo "$cmd"
    echo -e "${heredoc_contents[$((i-1))]}"
    echo "exit"
  } | ./minishell > logs/mini_out.txt 2>&1

  sed -i '' '/^exit$/d' logs/mini_out.txt 2>/dev/null || sed -i '/^exit$/d' logs/mini_out.txt

  if diff -u logs/bash_out.txt logs/mini_out.txt > logs/heredoc_fail_$i.diff ; then
    echo "✅ PASS"
    rm -f logs/heredoc_fail_$i.diff
  else
    echo "❌ FAIL (see logs/heredoc_fail_$i.diff)"
  fi
  ((i++))
done

rm -f logs/bash_out.txt logs/mini_out.txt
