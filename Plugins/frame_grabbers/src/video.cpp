#include "video.h"
#include "Aquila/Nodes/FrameGrabberInfo.hpp"
#include "boost/filesystem.hpp"
#include <ObjectInterfacePerModule.h>
using namespace aq;
using namespace aq::Nodes;
frame_grabber_video::~frame_grabber_video()
{
    StopThreads();
}
int frame_grabber_video::CanLoadDocument(const std::string& document)
{
    boost::filesystem::path path(document);
    auto extension = path.extension().string();
    return (extension == ".avi" || extension == ".mp4") ? 1 : 0;
}

rcc::shared_ptr<ICoordinateManager> frame_grabber_video::GetCoordinateManager()
{
    return coordinate_manager;
}

MO_REGISTER_CLASS(frame_grabber_video);