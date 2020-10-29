// CalculationDll.cpp : Defines the exported functions for the DLL application.
//
//#include "pch.h"
#include "include/Model.h"
#include "include/Tensor.h"

#include <numeric>
#include <iomanip>
#include"MathLibrary.h"
#include<iostream>

int Addition()
{
    // Load model with a path to the .pb file. 
    // An optional std::vector<uint8_t> parameter can be used to supply Tensorflow with
    // session options. The vector must represent a serialized ConfigProto which can be 
    // generated manually in python. See create_config_options.py.
    // Example:
    // const std::vector<uint8_t> ModelConfigOptions = { 0x32, 0xb, 0x9, 0x9a, 0x99, 0x99, 0x99, 0x99, 0x99, 0xb9, 0x3f, 0x20, 0x1 };
    // Model model("../model.pb", ModelConfigOptions);
    Model model("D:/git/cppflow/examples/load_model/model.pb");
    model.init();

    Tensor input_a{ model, "input_a" };
    Tensor input_b{ model, "input_b" };
    Tensor output{ model, "result" };

    std::vector<float> data(100);
    std::iota(data.begin(), data.end(), 0);

    input_a.set_data(data);
    input_b.set_data(data);

    model.run({ &input_a, &input_b }, output);
    for (float f : output.get_data<float>()) {
        std::cout << f << " ";
    }
    std::cout << std::endl;
	return 3;
}