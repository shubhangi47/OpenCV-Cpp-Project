#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>

std::vector<uchar> captureVideoFrames(const std::string &videoFileName)
{
    cv::VideoCapture capture(videoFileName);

    if (!capture.isOpened())
    {
        std::cerr << "Error: Unable to open video file." << std::endl;
        return std::vector<uchar>();
    }

    std::vector<uchar> encodedFrames;

    cv::Mat frame;
    while (capture.read(frame))
    {
        std::vector<uchar> encodedFrame;
        cv::imencode(".jpg", frame, encodedFrame);

        size_t frameSize = encodedFrame.size();
        encodedFrames.insert(encodedFrames.end(), reinterpret_cast<uchar*>(&frameSize), reinterpret_cast<uchar*>(&frameSize) + sizeof(size_t));
        encodedFrames.insert(encodedFrames.end(), encodedFrame.begin(), encodedFrame.end());
    }

    capture.release();
    return encodedFrames;
}

void replayFrames(const std::vector<uchar> &encodedFrames)
{
    cv::namedWindow("Reproduced Video", cv::WINDOW_NORMAL);

    size_t currentPosition = 0;

    while (currentPosition < encodedFrames.size())
    {
        size_t frameSize = *reinterpret_cast<const size_t*>(&encodedFrames[currentPosition]);
        currentPosition += sizeof(size_t);

        std::vector<uchar> frameData(encodedFrames.begin() + currentPosition, encodedFrames.begin() + currentPosition + frameSize);
        currentPosition += frameSize;

        cv::Mat decodedFrame = cv::imdecode(frameData, cv::IMREAD_COLOR);
        cv::imshow("Reproduced Video", decodedFrame);

        if (cv::waitKey(30) >= 0)
            break;
    }

    cv::destroyAllWindows();
}

int main()
{
    std::string inputVideoFile = "/home/Videos/input_video.mp4";
    std::string outputBinaryFile = "encoded_frames.bin";

    std::vector<uchar> encodedFrames = captureVideoFrames(inputVideoFile);

    if (encodedFrames.empty())
    {
        std::cerr << "Failed to capture video frames." << std::endl;
        return -1;
    }

    
    std::ofstream outFile(outputBinaryFile, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Error: Unable to open output file." << std::endl;
        return -1;
    }
    outFile.write(reinterpret_cast<const char *>(encodedFrames.data()), encodedFrames.size());
    outFile.close();

   
    std::ifstream inFile(outputBinaryFile, std::ios::binary);
    if (!inFile)
    {
        std::cerr << "Error: Unable to open input file." << std::endl;
        return -1;
    }
    inFile.seekg(0, std::ios::end);
    size_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);
    std::vector<uchar> readFrames(fileSize);
    inFile.read(reinterpret_cast<char *>(readFrames.data()), fileSize);

    replayFrames(readFrames);

    return 0;
}
