To compile use the command below in a terminal:
g++ <file path>

Which should result in a new file called a.exe in the same folder

Then to run the compiled code use the command with the correct file paths below:
<file path to a.exe> <file path to input file>


The runtime for my solution is m^2 + n^2 assuming there are m chargers and n availability reports
If I used a map or other similar data structure the run time would come closer to mlog(m) + nlog(n)