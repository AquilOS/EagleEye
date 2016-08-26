#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include "EagleLib/nodes/Node.h"
#include "EagleLib/nodes/ThreadedNode.h"
#include "EagleLib/IFrameGrabber.hpp"
#include "EagleLib/Logging.h"

#include "MetaObject/Parameters/ParameterMacros.hpp"
#include "MetaObject/Parameters/TypedInputParameter.hpp"
#include "MetaObject/MetaObjectFactory.hpp"
#include "MetaObject/Detail/MetaObjectMacros.hpp"
#include "MetaObject/MetaObjectFactory.hpp"
#include "MetaObject/Parameters/Buffers/StreamBuffer.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "EagleLibFrameGrabbers"
#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>


using namespace EagleLib;
using namespace EagleLib::Nodes;

struct test_framegrabber: public IFrameGrabber
{
    void ProcessImpl()
    {
        current.create(128,128, CV_8U);
        ++ts;
        current.setTo(ts);
        current_frame_param.UpdateData(current.clone(), ts, _ctx);
    }
    bool LoadFile(const std::string&)
    {
        return true;
    }
    int GetFrameNumber()
    {
        return ts;
    }
    int GetNumFrames()
    {
        return 255;
    }
    TS<SyncedMemory> GetCurrentFrame(cv::cuda::Stream& stream)
    {
        return TS<SyncedMemory>(current);
    }
    TS<SyncedMemory> GetNextFrame(cv::cuda::Stream& stream)
    {
        ++ts;
        Process();
        return GetCurrentFrame(stream);
    }
    TS<SyncedMemory> GetFrame(int ts, cv::cuda::Stream& stream)
    {
        cv::Mat output;
        output.create(128,128, CV_8U);
        output.setTo(ts);
        return output;
    }
    TS<SyncedMemory> GetFrameRelative(int offset, cv::cuda::Stream& stream)
    {
        cv::Mat output;
        output.create(128,128, CV_8U);
        output.setTo(ts + offset);
        return output;
    }
    rcc::shared_ptr<EagleLib::ICoordinateManager> GetCoordinateManager()
    {
        return rcc::shared_ptr<EagleLib::ICoordinateManager>();
    }

    MO_BEGIN(test_framegrabber, IFrameGrabber);
    MO_END;
    int ts = 0;
    cv::Mat current;
};

struct img_node: public Node
{
    MO_BEGIN(img_node, Node);
        INPUT(SyncedMemory, input, nullptr);
    MO_END;

    void ProcessImpl()
    {
        BOOST_REQUIRE(input);
        auto mat = input->GetMat(*(this->_ctx->stream));
        BOOST_REQUIRE_EQUAL(mat.at<uchar>(0), input_param.GetTimestamp());
    }
};

REGISTER_FRAMEGRABBER(test_framegrabber);
MO_REGISTER_CLASS(img_node);

BOOST_AUTO_TEST_CASE(test_dummy_output)
{
    EagleLib::SetupLogging();
    mo::MetaObjectFactory::Instance()->RegisterTranslationUnit();
    auto info = mo::MetaObjectFactory::Instance()->GetObjectInfo("test_framegrabber");
    bool val = std::is_base_of<EagleLib::Nodes::IFrameGrabber, test_framegrabber>::value;
    std::enable_if<std::is_base_of<EagleLib::Nodes::IFrameGrabber, test_framegrabber>::value, bool>::type test = true;
    BOOST_REQUIRE(info);
    
    
    auto fg = rcc::shared_ptr<test_framegrabber>::Create();
    auto node = rcc::shared_ptr<img_node>::Create();
    BOOST_REQUIRE(node->ConnectInput(fg, "input", "current_frame"));
    for(int i = 0; i < 100; ++i)
    {
        fg->Process();
    }
}