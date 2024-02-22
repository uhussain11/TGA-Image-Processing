#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
using namespace std;

#pragma pack(push, 1)
struct TGAHeader
{
    char idLength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;
    short xOrigin;
    short yOrigin;
    short width;
    short height;
    char pixelDepth;
    char imageDescriptor;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct TGAImage
{
    TGAHeader header;
    vector<uint8_t> data;
};
#pragma pack(pop)


TGAImage loadTGA(const string &filename)
{
    TGAImage image;
    ifstream file(filename, ios::binary);

    if (!file.is_open())
    {
        throw runtime_error("Failed to open the TGA file: " + filename);
    }

    // Read TGAHeader members individually
    file.read(&image.header.idLength, sizeof(image.header.idLength));
    file.read(&image.header.colorMapType, sizeof(image.header.colorMapType));
    file.read(&image.header.dataTypeCode, sizeof(image.header.dataTypeCode));
    file.read(reinterpret_cast<char *>(&image.header.colorMapOrigin), sizeof(image.header.colorMapOrigin));
    file.read(reinterpret_cast<char *>(&image.header.colorMapLength), sizeof(image.header.colorMapLength));
    file.read(&image.header.colorMapDepth, sizeof(image.header.colorMapDepth));
    file.read(reinterpret_cast<char *>(&image.header.xOrigin), sizeof(image.header.xOrigin));
    file.read(reinterpret_cast<char *>(&image.header.yOrigin), sizeof(image.header.yOrigin));
    file.read(reinterpret_cast<char *>(&image.header.width), sizeof(image.header.width));
    file.read(reinterpret_cast<char *>(&image.header.height), sizeof(image.header.height));
    file.read(&image.header.pixelDepth, sizeof(image.header.pixelDepth));
    file.read(&image.header.imageDescriptor, sizeof(image.header.imageDescriptor));

    unsigned int imageSize = image.header.width * image.header.height * (image.header.pixelDepth / 8);
    image.data.resize(imageSize);
    file.read(reinterpret_cast<char *>(image.data.data()), imageSize);

    file.close();
    return image;
}
/* Multiply Blend Method */
TGAImage blendImagesMultiply(const TGAImage &topLayer, const TGAImage &bottomLayer)
{
    TGAImage blendedImage;
    blendedImage.header = topLayer.header;
    unsigned int imageSize = topLayer.header.width * topLayer.header.height * (topLayer.header.pixelDepth / 8);
    blendedImage.data.resize(imageSize);

    int channels = topLayer.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        for (int j = 0; j < channels; ++j)
        {
            float topColor = static_cast<float>(topLayer.data[i + j]) / 255.0f;
            float bottomColor = static_cast<float>(bottomLayer.data[i + j]) / 255.0f;

            float blendedColor = topColor * bottomColor;
            blendedImage.data[i + j] = static_cast<uint8_t>(blendedColor * 255.0f);
        }
    }

    return blendedImage;
}

/* Screen Blend Method */
TGAImage blendImagesScreen(const TGAImage &topLayer, const TGAImage &bottomLayer)
{

    TGAImage blendedImage;
    blendedImage.header = topLayer.header;
    unsigned int imageSize = topLayer.header.width * topLayer.header.height * (topLayer.header.pixelDepth / 8);
    blendedImage.data.resize(imageSize);

    int channels = topLayer.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        for (int j = 0; j < channels; ++j)
        {
            float topColor = static_cast<float>(topLayer.data[i + j]) / 255.0f;
            float bottomColor = static_cast<float>(bottomLayer.data[i + j]) / 255.0f;

            float blendedColor = 1.0f - (1.0f - topColor) * (1.0f - bottomColor);
            blendedImage.data[i + j] = static_cast<uint8_t>(blendedColor * 255.0f);
        }
    }

    return blendedImage;
}

/* Subtract blend method */
TGAImage blendImagesSubtract(const TGAImage &topLayer, const TGAImage &bottomLayer)
{
    TGAImage blendedImage;
    blendedImage.header = topLayer.header;
    size_t imageSize = topLayer.header.width * topLayer.header.height * (topLayer.header.pixelDepth / 8);
    blendedImage.data.resize(imageSize);

    int channels = topLayer.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        for (int j = 0; j < channels; ++j)
        {
            int topColor = static_cast<int>(topLayer.data[i + j]);
            int bottomColor = static_cast<int>(bottomLayer.data[i + j]);

            int blendedColor = bottomColor - topColor;
            blendedColor = max(0, blendedColor);

            blendedImage.data[i + j] = static_cast<uint8_t>(blendedColor);
        }
    }

    return blendedImage;
}

/* Addition Blend Method */
TGAImage blendImagesAddition(const TGAImage &topLayer, const TGAImage &bottomLayer)
{

    TGAImage blendedImage;
    blendedImage.header = topLayer.header;
    size_t imageSize = topLayer.header.width * topLayer.header.height * (topLayer.header.pixelDepth / 8);
    blendedImage.data.resize(imageSize);

    int channels = topLayer.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        for (int j = 0; j < channels; ++j)
        {
            int topColor = static_cast<int>(topLayer.data[i + j]);
            int bottomColor = static_cast<int>(bottomLayer.data[i + j]);

            int blendedColor = bottomColor + topColor;
            blendedColor = min(255, blendedColor);

            blendedImage.data[i + j] = static_cast<uint8_t>(blendedColor);
        }
    }

    return blendedImage;
}

/* Overlay Blend Method */
TGAImage blendImagesOverlay(const TGAImage &topLayer, const TGAImage &bottomLayer)
{

    TGAImage blendedImage;
    blendedImage.header = topLayer.header;
    unsigned int imageSize = topLayer.header.width * topLayer.header.height * (topLayer.header.pixelDepth / 8);
    blendedImage.data.resize(imageSize);

    int channels = topLayer.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        float alpha = 1.0f;
        if (channels == 4) {
            alpha = static_cast<float>(topLayer.data[i + 3]) / 255.0f;
        }

        for (int j = 0; j < channels; ++j)
        {
            if (j == 3) {
                continue;
            }

            float topColor = static_cast<float>(topLayer.data[i + j]) / 255.0f;
            float bottomColor = static_cast<float>(bottomLayer.data[i + j]) / 255.0f;

            float blendedColor = 0.0f;
            if (bottomColor <= 0.5f)
            {
                blendedColor = 2.0f * topColor * bottomColor;
            }
            else
            {
                blendedColor = 1.0f - 2.0f * (1.0f - topColor) * (1.0f - bottomColor);
            }

            blendedColor = (alpha * blendedColor) + ((1.0f - alpha) * bottomColor); // Apply alpha blending
            blendedImage.data[i + j] = static_cast<uint8_t>(blendedColor * 255.0f);
        }
    }

    return blendedImage;
}

/* Add channel */
TGAImage addToChannel(const TGAImage &inputImage, char channel, int n)
{
    TGAImage modifiedImage;
    modifiedImage.header = inputImage.header;
    unsigned int imageSize = inputImage.header.width * inputImage.header.height * (inputImage.header.pixelDepth / 8);
    modifiedImage.data.resize(imageSize);

    int channels = inputImage.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        for (int j = 0; j < channels; ++j)
        {
            if ((j == 0 && channel == 'R') || (j == 1 && channel == 'G') || (j == 2 && channel == 'B'))
            {
                modifiedImage.data[i + j] = min(255, max(0, static_cast<int>(inputImage.data[i + j]) + n));
            }
            else
            {
                modifiedImage.data[i + j] = inputImage.data[i + j];
            }
        }
    }

    return modifiedImage;
}

/* Scale Channel */
TGAImage scaleChannel(const TGAImage &inputImage, char channel, float n)
{
    TGAImage modifiedImage;
    modifiedImage.header = inputImage.header;
    unsigned int imageSize = inputImage.header.width * inputImage.header.height * (inputImage.header.pixelDepth / 8);
    modifiedImage.data.resize(imageSize);

    int channels = inputImage.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        for (int j = 0; j < channels; ++j)
        {
            if ((j == 0 && channel == 'R') || (j == 1 && channel == 'G') || (j == 2 && channel == 'B'))
            {
                modifiedImage.data[i + j] = min(255.0f, max(0.0f, static_cast<float>(inputImage.data[i + j]) * n));
            }
            else
            {
                modifiedImage.data[i + j] = inputImage.data[i + j];
            }
        }
    }
    return modifiedImage;
}


/* Combine Channels */
TGAImage combineChannels(const TGAImage &redImage, const TGAImage &greenImage, const TGAImage &blueImage)
{
    TGAImage combinedImage;
    combinedImage.header = redImage.header;
    unsigned int imageSize = redImage.header.width * redImage.header.height * (redImage.header.pixelDepth / 8);
    combinedImage.data.resize(imageSize);

    int channels = redImage.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        combinedImage.data[i] = redImage.data[i];
        combinedImage.data[i + 1] = greenImage.data[i + 1];
        combinedImage.data[i + 2] = blueImage.data[i + 2];
    }

    return combinedImage;
}

TGAImage extractChannel(const TGAImage &inputImage, char channel)
{
    TGAImage extractedImage;
    extractedImage.header = inputImage.header;
    unsigned int imageSize = inputImage.header.width * inputImage.header.height * (inputImage.header.pixelDepth / 8);
    extractedImage.data.resize(imageSize);

    int channels = inputImage.header.pixelDepth / 8;

    for (unsigned int i = 0; i < imageSize; i += channels)
    {
        for (int j = 0; j < channels; ++j)
        {
            if ((j == 0 && channel == 'R') || (j == 1 && channel == 'G') || (j == 2 && channel == 'B'))
            {
                extractedImage.data[i + j] = inputImage.data[i + j];
            }
            else
            {
                extractedImage.data[i + j] = 0;
            }
        }
    }
    return extractedImage;
}

/* Rotate 180 Degrees */
TGAImage rotate180(const TGAImage &inputImage)
{
    TGAImage rotatedImage;
    rotatedImage.header = inputImage.header;
    unsigned int imageSize = inputImage.header.width * inputImage.header.height * (inputImage.header.pixelDepth / 8);
    rotatedImage.data.resize(imageSize);

    int channels = inputImage.header.pixelDepth / 8;

    for (int y = 0; y < inputImage.header.height; ++y)
    {
        for (int x = 0; x < inputImage.header.width; ++x)
        {
            unsigned int srcIndex = (y * inputImage.header.width + x) * channels;
            unsigned int dstIndex = ((inputImage.header.height - y - 1) * inputImage.header.width + (inputImage.header.width - x - 1)) * channels;

            for (int c = 0; c < channels; ++c)
            {
                rotatedImage.data[dstIndex + c] = inputImage.data[srcIndex + c];
            }
        }
    }

    return rotatedImage;
}

void saveTGA(const string &filename, const TGAImage &image)
{
    ofstream file(filename, ios::binary);

    if (!file.is_open())
    {
        throw runtime_error("Error: Could not open file: " + filename);
    }

    file.write(reinterpret_cast<const char *>(&image.header), sizeof(TGAHeader));

    unsigned int imageSize = image.header.width * image.header.height * (image.header.pixelDepth / 8);
    file.write(reinterpret_cast<const char *>(image.data.data()), imageSize);

    file.close();
}

int main(int argc, char *argv[]) {
    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        cout << "Project 2: Image Processing, Spring 2023\n"
                "Usage:\n"
                "    ./project2.out [output] [firstImage] [method] [...]\n";
        return 0;
    }

    string outputFilename(argv[1]);
    string firstImageFilename(argv[2]);

    TGAImage currentImage = loadTGA(firstImageFilename);

    bool firstOperation = true;
    for (int i = 3; i < argc; i++) {
        string method = argv[i];

        if (method == "multiply" && i + 1 < argc) {
            i++;
            TGAImage otherImage = loadTGA(argv[i]);
            currentImage = blendImagesMultiply(currentImage, otherImage);

            if (firstOperation) {
                firstOperation = false;
                cout << "Multiplying " << firstImageFilename << " and " << argv[i] << " ...\n";
            } else {
                cout << "... and multiplying " << argv[i] << " with result of previous step  ...\n";
            }
        } else if (method == "subtract" && i + 1 < argc) {
            i++;
            TGAImage otherImage = loadTGA(argv[i]);
            currentImage = blendImagesSubtract(currentImage, otherImage);

            if (firstOperation) {
                firstOperation = false;
                cout << "Subtracting " << firstImageFilename << " and " << argv[i] << " ...\n";
            } else {
                cout << " ... and subtracting " << argv[i] << " from previous step ...\n";
            }
        } else if (method == "overlay" && i + 1 < argc) {
            i++;
            TGAImage otherImage = loadTGA(argv[i]);
            currentImage = blendImagesOverlay(currentImage, otherImage);

            if (firstOperation) {
                firstOperation = false;
                cout << "Overlaying " << firstImageFilename << " and " << argv[i] << " ...\n";
            } else {
                cout << " ... and overlaying " << argv[i] << " with result of previous step ...\n";
            }
        } else if (method == "screen" && i + 1 < argc) {
            i++;
            TGAImage otherImage = loadTGA(argv[i]);
            currentImage = blendImagesScreen(currentImage, otherImage);
            cout << "Screen blending " << firstImageFilename << " and " << argv[i] << " ...\n";
        } else if (method == "combine") {
            if (i + 2 > argc - 1) {
                cout << "Error: Not enough input files for combine operation.\n";
                return 1;
            }

            string greenImageFilename = argv[i + 1];
            string blueImageFilename = argv[i + 2];

            TGAImage greenImage = loadTGA(greenImageFilename);
            TGAImage blueImage = loadTGA(blueImageFilename);

            currentImage = combineChannels(currentImage, greenImage, blueImage);

            i += 2;

            if (firstOperation) {
                firstOperation = false;
                cout << "Combining channels from " << "running image, " << greenImageFilename << ", and "
                     << blueImageFilename << " ...\n";
            } else {
                cout << " ... and combining channels from running image " << " , " << greenImageFilename << ", and "
                     << blueImageFilename << " to previous step ...\n";
            }
        } else if (method == "flip") {
            currentImage = rotate180(currentImage);

            if (firstOperation) {
                firstOperation = false;
                cout << "Flipping " << firstImageFilename << " ...\n";
            } else {
                cout << " ... and flipping output of previous step ...\n";
            }
        } else if (method == "onlyred") {
            currentImage = extractChannel(currentImage, 'R');

            if (firstOperation) {
                firstOperation = false;
                cout << "Only Red " << firstImageFilename << " ...\n";
            } else {
                cout << " ... and getting only red output of previous step ...\n";
            }
        } else if (method == "onlygreen") {
            currentImage = extractChannel(currentImage, 'G');

            if (firstOperation) {
                firstOperation = false;
                cout << "Only Green " << firstImageFilename << " ...\n";
            } else {
                cout << " ... and getting only green output of previous step ...\n";
            }
        } else if (method == "onlyblue") {
            currentImage = extractChannel(currentImage, 'B');

            if (firstOperation) {
                firstOperation = false;
                cout << "Only Blue " << firstImageFilename << " ...\n";
            } else {
                cout << " ... and getting only blue output of previous step ...\n";
            }
        } else if (method == "addred") {
            if (i + 1 < argc) {
                int amount = stoi(argv[i + 1]);
                TGAImage modifiedImage = addToChannel(currentImage, 'R', amount);
                currentImage = modifiedImage;
                i++;

                if (firstOperation) {
                    firstOperation = false;
                    cout << "Adding " << amount << " to the red channel of " << firstImageFilename << " ...\n";
                } else {
                    cout << " ... and adding " << amount << " to the red channel of previous step ...\n";
                }
            } else {
                cout << "Error: Missing amount to add to the red channel.\n"; //fix
                return 1;
            }
        } else if (method == "addgreen") {
            if (i + 1 < argc) {
                int amount = stoi(argv[i + 1]);
                TGAImage modifiedImage = addToChannel(currentImage, 'G', amount);
                currentImage = modifiedImage;
                i++;

                if (firstOperation) {
                    firstOperation = false;
                    cout << "Adding " << amount << " to the green channel of " << firstImageFilename << " ...\n";
                } else {
                    cout << " ... and adding " << amount << " to the green channel of previous step ...\n";
                }
            } else {
                cout << "Error: Missing amount to add to the green channel.\n";
                return 1;
            }
        } else if (method == "addblue") {
            if (i + 1 < argc) {
                int amount = stoi(argv[i + 1]);
                TGAImage modifiedImage = addToChannel(currentImage, 'B', amount);
                currentImage = modifiedImage;
                i++;

                if (firstOperation) {
                    firstOperation = false;
                    cout << "Adding " << amount << " to the blue channel of " << firstImageFilename << " ...\n";
                } else {
                    cout << " ... and adding " << amount << " to the blue channel of previous step ...\n";
                }
            } else {
                cout << "Error: Missing amount to add to the blue channel.\n";
                return 1;
            }
        } else if (method == "scalered") {
            if (i + 1 < argc) {
                float amount = stof(argv[i + 1]);
                TGAImage modifiedImage = scaleChannel(currentImage, 'R', amount);
                currentImage = modifiedImage;
                i++;

                if (firstOperation) {
                    firstOperation = false;
                    cout << "Scaling " << amount << " to the red channel of " << firstImageFilename << " ...\n";
                } else {
                    cout << " ... and scaling " << amount << " to the red channel of previous step ...\n";
                }
            } else {
                cout << "Error: Missing amount to scale to the red channel.\n";
                return 1;
            }
        } else if (method == "scalegreen") {
            if (i + 1 < argc) {
                float amount = stof(argv[i + 1]);
                TGAImage modifiedImage = scaleChannel(currentImage, 'G', amount);
                currentImage = modifiedImage;
                i++;

                if (firstOperation) {
                    firstOperation = false;
                    cout << "Scaling " << amount << " to the green channel of " << firstImageFilename << " ...\n";
                } else {
                    cout << " ... and scaling " << amount << " to the green channel of previous step ...\n";
                }
            } else {
                cout << "Error: Missing amount to scale to the green channel.\n";
                return 1;
            }
        } else if (method == "scaleblue") {
            if (i + 1 < argc) {
                float amount = stof(argv[i + 1]);
                TGAImage modifiedImage = scaleChannel(currentImage, 'B', amount);
                currentImage = modifiedImage;
                i++;

                if (firstOperation) {
                    firstOperation = false;
                    cout << "Scaling " << amount << " to the blue channel of " << firstImageFilename << " ...\n";
                } else {
                    cout << " ... and scaling " << amount << " to the blue channel of previous step ...\n";
                }
            } else {
                cout << "Error: Missing amount to scale to the blue channel.\n";
                return 1;
            }
        } else {
            cout << "Invalid method or missing arguments: " << method << endl;
            return 1;
        }
    }

    saveTGA(outputFilename, currentImage);
    cout << "... and saving output to " << outputFilename << "!\n";


    return 0;
}