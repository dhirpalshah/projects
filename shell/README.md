shell project
why use the terminal when you can run another terminal inside the terminal?

how it works:
learned a lot about systems programming in one of my intro classes,
and they wanted us to write a shell that could run 3 basic commands.
"cd", "ls", and "pwd". I decided to implement some more functions

this version can run redirection and most basic commands of a terminal.
2 most important parts of this project was learning how to parse and
required a good understanding of processes / forking

interesting stuff:
execve is the commmand that most terminals use when calling general commands
in order to maintain efficiency, we need to fork whenever execve is called
so that the other processes can continue running. pretty cool.