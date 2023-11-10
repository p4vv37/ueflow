# ueflow
TensorFlow model used to generate a water surface around a moving obstacle in an Unreal Engine 4.
Based on cppflow - https://github.com/serizba/cppflow

## Example
Video demonstration (youtube):

[![Demo](http://img.youtube.com/vi/oB-kbE85IRU/0.jpg)](https://www.youtube.com/watch?v=oB-kbE85IRU)

![Preview](examples/simple.gif)
![Preview](examples/complex.gif)

![Part of thesis PL](http://www.pkowalski.com/wp_portfolio/wp-content/uploads/2023/11/skrot_mgr.pdf)

![Part of thesis EN](http://www.pkowalski.com/wp_portfolio/wp-content/uploads/2022/02/short_EN.pdf)

![Thesis PL](http://www.pkowalski.com/wp_portfolio/wp-content/uploads/2023/11/Paweł_Kowalski-Głębokie_sieci_w_grach_m_2023-2.pdf)

## Requirements
Requirements:
CUDNN 8.1
CUDA 11.2

`https://github.com/serizba/cppflow/tree/cppflow2/include/cppflow` need to be placed in `TFInUnreal\Source\ThirdParty\include`

Tensorflow C API libraries are needed. They can be downloaded from:
https://www.tensorflow.org/install/lang_c
And should be placed in:
`TFInUnreal\Source\ThirdParty\`
As an example, path to tensorflow.dll should like like this:
`TFInUnreal\Source\ThirdParty\lib\tensorflow.dll`

Both GPU and CPU versions should work, if versions of CUDNN and CUDA are correct.

Unreal treats warning raised in cppflow as an error, so comment lines 942-944:

``` c++
TF_CAPI_EXPORT extern TF_WhileParams TF_NewWhile(TF_Graph* g, TF_Output* inputs,
                                                 int ninputs,
                                                 TF_Status* status);
```

in `tensorflow\c\c_api.h`

In the same file add following include:
`#include <iterator>`
