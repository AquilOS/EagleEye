#include "nodes/Root.h"
#include <boost/chrono.hpp>
#include <boost/date_time.hpp>

using namespace EagleLib;

Root::Root()
{
    objSystem.reset(new RuntimeObjectSystem());
}

Root::~Root()
{

}

cv::cuda::GpuMat
Root::doProcess(cv::cuda::GpuMat& img)
{
    static boost::posix_time::ptime prevTime = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration delta = boost::posix_time::microsec_clock::universal_time() - prevTime;
    objSystem->GetFileChangeNotifier()->Update(delta.total_milliseconds());

    if(objSystem->GetIsCompiling() && statusCallback)
            statusCallback(std::string("Recompiling"));
    if(objSystem->GetIsCompiledComplete())
        objSystem->LoadCompiledModule();
    for(int i = 0; i < children.size(); ++i)
    {
        img = children[i]->process(img);
    }
    return img;
}
