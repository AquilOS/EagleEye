#include "RosPublisher.hpp"
#include "RosInterface.hpp"
#include "Aquila/utilities/CudaCallbacks.hpp"
#include "Aquila/Nodes/NodeInfo.hpp"

#include <ros/node_handle.h>
#include <cv_bridge/cv_bridge.h>


using namespace aq;
using namespace aq::Nodes;


void ImagePublisher::NodeInit(bool firstInit)
{
    _image_publisher = RosInterface::Instance()->nh()->advertise<sensor_msgs::Image>(topic_name, 1);
}

bool ImagePublisher::ProcessImpl()
{
    // Check where the data resides
    SyncedMemory::SYNC_STATE state = input->GetSyncState();
    // Download to the CPU
    cv::Mat h_input = input->GetMat(Stream());
    // If data is already on the cpu, we don't need to wait for the async download to finish
    if(state < SyncedMemory::DEVICE_UPDATED)
    {
        // data already updated on cpu
        sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", h_input).toImageMsg();
        _image_publisher.publish(msg);
        ros::spinOnce();
    }else
    {
        // data on gpu, do async publish
        size_t id = mo::GetThisThread();
        aq::cuda::enqueue_callback_async(
            [h_input, this]()->void
        {
            // This code is executed on cpu thead id after the download is complete
            sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", h_input).toImageMsg();
            _image_publisher.publish(msg);
        }, id, Stream());
    }
    return true;
}

MO_REGISTER_CLASS(ImagePublisher)
