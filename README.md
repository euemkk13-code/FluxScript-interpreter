# FluxScript-interpreter
FluxScript is an indie PL that was created today, october 24, its simple  and fast, i wish that in future versions FluxScript can make many games  and apps and be popular, not for fame, but for people can code something  with less strugling.

Currently is in version 0.2.0, only math, print, variable and conditionals. 
## Tutorial



### 1 You use out for text printing, like print("") in python. but you dont need parenthesys.
Example: out "test" // you can also use ' for strings

### 2 You use // for single line comments and --[[]] for mutiple line comments
Example: 

// this is a comment, i can use it for only one line

putting this text without // into the file will cause an error

--[[ this is a multiple line comment,

i can use it in multiple lines
hello

1

2

3

the comment will only end when i type ]]

### 3 You can make basic math, only four operators and PEMDAS parenthesys.
Example:

out 5 // This will print 5.

out 5 + 5 // This will print 10.

out 5 - 5 // This will print 0.

out 5 * 5 // This will print 25.

out 5 / 5 // This will print 1.

out 5 + 5 / 5 // This will print 6.

out (5 + 5) / 5 // This will print 2.

### 4 IF / ELSE
Is checking condition. If true, do then. If false, do else.

Syntax:

if (condition) {

  out "something"
  
} else {

  out "other thing"
  
}Else is optional. You can remove.

Example:

x = 10

if (x > 5) {

  out "big"
  
}Operators: ==  !=  <  >  <=  >=  and  or

### WHILE

Is looping. While condition is true, repeat.

Syntax:

while (condition) {

  // do stuff
  
}Example - counting:

i = 0

while (i < 3) {

  out i
  
  i = i + 1
  
}This should print 0, 1, 2.

COMBINED

i = 0

while (i < 5) {

  if (i == 2) {
  
    out "two found"
    
  }
  
  i = i + 1
  
}IMPORTANT: Always use { } for block. Always use ( ) for condition.


That's it!
