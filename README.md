**Usage:** 

`./hklridx filename.HKL 100 010 001`

 or `010 100 001` (to flip h and k)
 
 or `-100 001 010` (to flip k and l, while inverting h)

 more complicated matrices different than the unity can be given too.



**Download:**
- `git clone https://github.com/m92rizk/reindex_hkl.git`


**Dependencies:**

include iostream
include fstream
include string
include algorithm
include array
include vector
include chrono
include sstream
include unordered_map
include utility
include cmath
include iomanip
include numeric

**Compiling:**
- linux: `g++ reindex_HKLs.cpp  -o hklridx -std=c++11`
- windows:
- 1) need to install: `winget install Microsoft.VisualStudio.2022.BuildTools --force --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.26100"`
  2) Start Menu → Visual Studio → "Developer Command Prompt for Visual Studio" (not the regular cmd)
  3) compile: `cl /MD	/EHsc reindex_HKLs.cpp`
