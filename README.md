# ueflow
TensorFlow model used to generate a water surface around a moving obstacle in an Unreal Engine 4.

Video demonstration (youtube):

[![Demo](http://img.youtube.com/vi/oB-kbE85IRU/0.jpg)](https://www.youtube.com/watch?v=oB-kbE85IRU)

![Preview](examples/simple.gif)
![Preview](examples/complex.gif)

Based on cppflow - https://github.com/serizba/cppflow

Requirements:
CUDNN 8.1
CUDA 11.2

Tensorflow C API libraries are needed. They can be downloaded from:
https://www.tensorflow.org/install/lang_c
And should be placed in:
TFInUnreal/Source/ThirdParty/libtensorflow
As an example, path to tensorflow.dll should like like this:
TFInUnreal\Source\ThirdParty\libtensorflow\lib\tensorflow.dll

Both GPU and CPU versions should work, if versions of CUDNN and CUDA are correct.

Unreal treats warning raised in cppflow as an error, so comment lines 942-944:

TF_CAPI_EXPORT extern TF_WhileParams TF_NewWhile(TF_Graph* g, TF_Output* inputs,
                                                 int ninputs,
                                                 TF_Status* status);

in 
tensorflow\c\c_api.h

This error prevented everything from working:
https://github.com/serizba/cppflow/issues/87

Work in progress.
