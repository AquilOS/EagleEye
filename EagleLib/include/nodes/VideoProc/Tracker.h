#pragma once

#include <nodes/Node.h>
#include "Tracking.hpp"
#include <boost/circular_buffer.hpp>
#include "CudaUtils.hpp"
namespace EagleLib
{

    class KeyFrameTracker: public Node
    {
        // Used to find homography once the data has been downloaded from the stream
        ConstEventBuffer<TrackingResults> homographyBuffer;
        boost::circular_buffer<TrackedFrame> trackedFrames;
        std::map<int, KeyFrame> keyFrames;

    public:
        KeyFrameTracker();
        virtual void Init(bool firstInit);
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream& stream = cv::cuda::Stream::Null());
    };

    class CMTTracker: public Node
    {

    public:
        CMTTracker();
        virtual void Init(bool firstInit);
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream& stream = cv::cuda::Stream::Null());

    };

    class TLDTracker:public Node
    {
    public:
        TLDTracker();
        virtual void Init(bool firstInit);
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat &img, cv::cuda::Stream& stream = cv::cuda::Stream::Null());
    };
}
