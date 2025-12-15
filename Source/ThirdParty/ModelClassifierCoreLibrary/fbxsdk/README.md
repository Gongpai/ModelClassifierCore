\# FBX SDK (User-Provided Dependency)



This project requires \*\*Autodesk FBX SDK version 2020.3.7\*\*  

Due to licensing restrictions, the FBX SDK \*\*is NOT redistributed\*\* with this repository.



Users must download the SDK manually and agree to Autodesk's license terms.



---



\## Required Version



\- \*\*FBX SDK:\*\* 2020.3.7



Other versions may work but are \*\*not officially supported\*\* and have not been tested with this project.



---



\## Download Instructions



1\. Visit the official Autodesk FBX SDK page:  

&nbsp;  https://aps.autodesk.com/developer/overview/fbx-sdk



2\. Download \*\*FBX SDK 2020.3.7\*\* for your operating system.



3\. During installation or extraction, review and \*\*accept the Autodesk FBX SDK License Agreement\*\*.



---



\## Installation



After downloading the FBX SDK:



\- Place the SDK files inside this `fbxsdk/` directory  

&nbsp; or configure your build system to point to the installed FBX SDK location.



The expected structure may look like:

fbxsdk/

├─ include/

├─ lib/

└─ samples/



---



\## License Notice



Autodesk FBX SDK is proprietary software licensed by Autodesk, Inc.



\- This repository does \*\*not\*\* contain any FBX SDK files.

\- This project is \*\*not affiliated with or endorsed by Autodesk\*\*.

\- Use of the FBX SDK is subject to Autodesk's own license terms.



By downloading and using the FBX SDK, you agree to comply with Autodesk's license agreement.



---



\## Troubleshooting



If the build system cannot find the FBX SDK:



\- Verify that the SDK version is \*\*2020.3.7\*\*

\- Check include and library paths in your build configuration

\- Ensure environment variables (if required) are correctly set



---



