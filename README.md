# ueflow
Use TensorFlow models in an Unreal Engine 4 blueprint.
Repository is made out of 2 projects: one creates .dll file, the second one is an Unreal project.

Execute build_dll.bat to compile then copy library and model to the right place in Unreal project source code. This bat is also executed in Unreal project build.cs

To run the network place node LoadAndRunNetwork in blueprint

Based on cppflow - https://github.com/serizba/cppflow


Tensorflow C API libraries are needed. They can be downloaded from:
https://www.tensorflow.org/install/lang_c
And should be placed in:
model2dll/ThirdParty/libtensorflow
As an example, path to tensorflow.dll should like like this:
model2dll\ThirdParty\libtensorflow\lib\tensorflow.dll


Work in progress.
