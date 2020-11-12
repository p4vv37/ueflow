// TFLibrary.cpp : Defines the exported functions for the DLL application.
#include "TFLibrary.h"


int ExecuteExample()
{
    Model model("model.pb");
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


bool TFNetwork::Initialize(const int& numberOfFrames, const int& numberOfBlocks, const char* modelPath)
{
    // Tired of crashes, just chec iffile exists.
    std::ifstream f(modelPath);
    if (!f.good())
        return false;

    m_numberOfFrames = numberOfFrames;
    m_numberOfBlocks = numberOfBlocks;

    m_model = std::unique_ptr<Model>(new Model(modelPath));
    m_model->init();

    m_input = std::unique_ptr<Tensor>(new Tensor(*m_model, "input"));
    m_result = std::unique_ptr<Tensor>(new Tensor(*m_model, "result"));

    return true;
}


bool TFNetwork::AddSample(const float* positions, const float* orientations)
{
    m_positions.addSample(positions);
    m_orientations.addSample(orientations);
    return true;
}


bool TFNetwork::Predict(float* positions, float* orientations)
{
    for (int i = 0; i < m_numberOfBlocks; i++)
    {
        if (i % 3)
        {
            continue;
        }
        positions[i] += 0.1;
    }
    return true;
}