# Commands
Before running any commands, make sure your source code is located in the `scratch` file in `ns3`.
## Run Project
Run simulation:
```
./waf --run scratch/your_file_name
```
Run and save output files in scratch:
```
./waf --run scratch/your_file_name --cwd scratch
```

## Run NetAnim
This command may run once NetAnim is comletely installed according to this !(link)[https://www.nsnam.org/wiki/NetAnim_3.108].
Navigate to netanim :
```
 cd netanim/
 ./NetAnim
```
Run NetAnim:
```
 ./NetAnim
```
## Run Debugger
Run debugger:
```
./waf --run part3 --command-template="kdbg %s"
```
