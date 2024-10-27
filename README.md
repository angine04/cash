# cash - Yet another hilarious toy shell project

## Introduction

cash, short for **Can't Afford a SHell**, is a toy shell project aimed at hilarity and amusement. (That's exactly what a TOY is meant to do.)

It's written by me (yes, me, but who am I?) during an annoying afternoon, as I am just told that this assignment has to be submitted tonight.

Anyway, should you accept it or not, may you have a great time and enjoy this toy.

by Dr. _Awkward Nutty Goofy Idiotic Notorious Eccentric_

## Features

 - Runs commands that you input
 - Arguments can have spaces in them if you use quotation marks `""`
 - Built-in commands
   - history: Lists all commands you've used
   - cd: Changes directory
   - help: Prints help message
   - exit: Exits the shell (Try Ctrl+D also!)
 - You can use pipes (one at a time)
   
   Here's some test suites if you'd like to have some:
   - `echo "Hello cash" | grep cash`
   - `echo "C A S H" | tr " " "\n"`
   - `ls | nonexistent_command`
   - `ls | wc -l`