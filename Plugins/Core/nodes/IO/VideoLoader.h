#include <nodes/Node.h>
#include <external_includes/cv_videoio.hpp>
#include <external_includes/cv_cudacodec.hpp>
#include "RuntimeInclude.h"
#include "RuntimeSourceDependency.h"
RUNTIME_COMPILER_SOURCEDEPENDENCY
RUNTIME_MODIFIABLE_INCLUDE
#include "CudaUtils.hpp"
#include <boost/thread.hpp>
namespace EagleLib
{

    class  VideoLoader: public Node
    {

        concurrent_notifier<cv::cuda::GpuMat> notifier;

        ConstBuffer<cv::cuda::HostMem> h_img;
        bool load;
        boost::thread readThread;
    public:
        void ReadThread();
        VideoLoader();
        ~VideoLoader();
        void Init(bool firstInit);
        void loadFile();
        void restartVideo();
        virtual void Serialize(ISimpleSerializer *pSerializer);
        virtual bool SkipEmpty() const;
        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat& img, cv::cuda::Stream& stream = cv::cuda::Stream::Null());
        cv::Ptr<cv::cudacodec::VideoReader> d_videoReader;
        cv::Ptr<cv::VideoCapture>           h_videoReader;

    };
}
