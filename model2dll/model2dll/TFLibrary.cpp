// TFLibrary.cpp : Defines the exported functions for the DLL application.
#include "TFLibrary.h"


namespace utils {
    float GetAngle(const float& sin, const float& cos) 
    {
        return atan2(sin, cos) * 180.0f / PI;
    }

    void GetSinCos(const float& angle, float& s, float& c)
    {
        s = sin(angle);
        c = cos(angle);
    }

    std::vector<std::string> parseLine(const std::string& line) {
        std::vector<std::string> result;

        std::stringstream s_stream(line.c_str()); // strcpy() is unsafe error on s_stream.good()
        while (s_stream.good()) {
            std::string substr;
            getline(s_stream, substr, ';');
            result.push_back(substr);
        }
        return result;
    }
} // utils


//
//  Simple network usage example
//

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

//
//  SamplesCache
//

SamplesCache::SamplesCache(const SamplesCache& other) 
{
    m_numberOfElements = other.m_numberOfElements;
    m_numberOfSamples = other.m_numberOfSamples;
    UpdateSize();
    std::copy(other.m_data, other.m_data + other.m_numberOfElements * other.m_numberOfSamples, m_data);
};

SamplesCache::SamplesCache(SamplesCache&& other) : m_data{ other.m_data }, m_numberOfElements{ other.m_numberOfElements }, m_numberOfSamples{ other.m_numberOfSamples } 
{ 
    other.m_data = nullptr; 
};

SamplesCache& SamplesCache::operator=(const SamplesCache& other) 
{ 
    if (&other != this) 
    { 
        delete[] m_data;
        std::copy(other.m_data, other.m_data + other.m_numberOfElements * other.m_numberOfSamples, m_data);
    }       
    return *this; 
};
SamplesCache& SamplesCache::operator=(SamplesCache&& other) 
{ 
    if (&other != this) 
    { 
        delete[] m_data;
        m_data = other.m_data;
        other.m_data = nullptr;
    }       
    return *this; 
};

SamplesCache::~SamplesCache()
{
    delete[] m_data;
}

void SamplesCache::UpdateSize()
{
    if (m_data)
    {
        delete[] m_data;
    }
    m_data = new float[m_numberOfSamples* m_numberOfElements];
    m_headIndex = 0;
}

void SamplesCache::SetNumberOfSamples(const int numberOfSamples)
{
    m_numberOfSamples = numberOfSamples;
    UpdateSize();
}
void SamplesCache::SetNumberOfElements(const int numberOfElements)
{
    m_numberOfElements = numberOfElements;
    UpdateSize();
}
void SamplesCache::addSample(const float* sample)
{
    m_headIndex++;
    if (m_headIndex >= m_numberOfSamples)
    {
        m_headIndex = 0;
    }
    memcpy(&m_data[m_headIndex], sample, sizeof(float) * m_numberOfElements);
}

//
//  TFNetwork
//

TFNetwork::~TFNetwork() {
    // delete m_model; // Leak or crash? Leak for now.
    delete m_initialBlocksOrientations;
    delete m_initialBlocksPositions;
    delete m_blocksModels;
}

bool TFNetwork::Initialize(int& numberOfFrames, int& numberOfBlocks, const char* modelPath)
{
    // Tired of crashes, just chec iffile exists.
    std::ifstream f(modelPath);
    if (!f.good())
        return false;

    m_model = new Model(modelPath);
    m_model->init();

    std::string cgPath{ modelPath };
    cgPath  = cgPath.substr(0, cgPath.find_last_of('.')) + ".cfg";
    std::ifstream file(cgPath);
    if (!file.good())
        return false;

    std::string line;

    // First line is: <number of frames>;<number of blocks>
    std::getline(file, line);
    std::vector<std::string> parsedLine = utils::parseLine(line);
    m_numberOfFrames = std::stoi(parsedLine[0]);
    m_numberOfBlocks = std::stoi(parsedLine[1]);
    numberOfFrames = m_numberOfFrames;
    numberOfBlocks = m_numberOfBlocks;

    m_positions.SetNumberOfElements(numberOfBlocks);
    m_positions.SetNumberOfSamples(numberOfFrames);

    m_orientations.SetNumberOfElements(numberOfBlocks);
    m_orientations.SetNumberOfSamples(numberOfFrames);

    // All other lines: <block model relative path>;<pos x>;<pos y>;<pos z>;<sin rotation x><cos rotation x>;<sin rotation y><cos rotation y>;<sin rotation z><cos rotation z>;
    float angle;
    std::vector<float> initialBlocksPositions;
    std::vector<float> initialBlocksOrientations;
    std::vector<std::string> blocksModels;
    std::vector<char> array;
    array.push_back('c');
    while (std::getline(file, line)) {
        parsedLine = utils::parseLine(line);
        std::string model(parsedLine[0]);
        blocksModels.push_back(model);
        initialBlocksPositions.push_back(std::stof(parsedLine[1]));
        initialBlocksPositions.push_back(std::stof(parsedLine[2]));
        initialBlocksPositions.push_back(std::stof(parsedLine[3]));

        for (std::size_t num{ 0 }; num < 3; num++)
        {
            angle = utils::GetAngle(std::stof(parsedLine[4 + num * 2]), std::stof(parsedLine[5 + num * 2])); 
            initialBlocksOrientations.push_back(angle);
        }
    }
    file.close();

    m_initialBlocksPositions = new float[initialBlocksPositions.size()];
    std::copy(initialBlocksPositions.begin(), initialBlocksPositions.begin() + initialBlocksPositions.size(), m_initialBlocksPositions);
    m_initialBlocksOrientations = new float[initialBlocksOrientations.size()];
    std::copy(initialBlocksOrientations.begin(), initialBlocksOrientations.begin() + initialBlocksOrientations.size(), m_initialBlocksOrientations);

    std::string  resultData;
    for (std::string modelPath : blocksModels)
    {
        resultData.append(modelPath + ";");
    }

    m_blocksModels = new char[resultData.size() + 1];
    resultData.copy(m_blocksModels, resultData.size());
    m_blocksModels[resultData.size()] = '\0';

    if (blocksModels.size() != m_numberOfBlocks)
    {
        return false;
    }

    m_input = new Tensor(*m_model, "input_a");
    m_result = new Tensor(*m_model, "result");

    return true;
}

const char* TFNetwork::getBlocksModels()
{
    return m_blocksModels;
}

void TFNetwork::getInitialPositions(float* positions)
{
    std::copy(m_initialBlocksPositions, m_initialBlocksPositions + m_numberOfBlocks, positions);
}

void TFNetwork::getInitialOrientations(float* orientations)
{
    std::copy(m_initialBlocksOrientations, m_initialBlocksOrientations + m_numberOfBlocks, orientations);
}

bool TFNetwork::AddSample(const float* positions, const float* orientations)
{
    m_positions.addSample(positions);
    m_orientations.addSample(orientations);
    return true;
}


bool TFNetwork::Predict(float* positions, float* orientations)
{
    // Dummy implementation
    for (int i = 0; i < m_numberOfBlocks; i++)
    {
        if (i % 3)
        {
            continue;
        }
        positions[i] += 0.1f;
        orientations[i] += 0.1f;
    }
    return true;
}