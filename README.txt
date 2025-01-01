To compile use the command below in a terminal:
g++ <file path>

Which should result in a new file called a.exe in the same folder

Then to run the compiled code use the command with the correct file paths below:
<file path to a.exe> <file path to input file>


The runtime for my solution is n assuming there n lines in the file
It may be longer depending on the implementation of certain library functions

I added 3 new test cases:
- input_error_1.txt has a string instead of a number
- input_error_2.txt has an invalid time sequence i.e. it starts at a later time than it ends
- input_error_3.txt has a charger at 2 different stations