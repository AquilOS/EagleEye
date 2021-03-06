#include "cv_capture.h"
#include "precompiled.hpp"
#if _MSC_VER
RUNTIME_COMPILER_LINKLIBRARY("ole32.lib")
#endif
using namespace aq;
using namespace aq::Nodes;

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


frame_grabber_cv::frame_grabber_cv():
    FrameGrabberThreaded()
{
    playback_frame_number = -1;
}

bool frame_grabber_cv::LoadFile(const std::string& file_path)
{
    if(d_LoadFile(file_path))
    {
        return true;
    }else
    {
        return h_LoadFile(file_path);
    }
    return false;
}

bool frame_grabber_cv::d_LoadFile(const std::string& file_path)
{
    d_cam.release();
    try
    {
        auto d_temp = cv::cudacodec::createVideoReader(file_path);
        if (d_temp)
        {
            d_cam = d_temp;
            loaded_document = file_path;
            return true;
        }
    }
    catch (cv::Exception& e)
    {

    }
    return false;
}

bool frame_grabber_cv::h_LoadFile(const std::string& file_path)
{
    h_cam.release();
    //LOG(info) << "Attemping to load " << file_path;
    BOOST_LOG_TRIVIAL(info ) << "[" << GetTreeName() << "::h_loadFile] Trying to load: \"" << file_path << "\"";
    boost::mutex::scoped_lock lock(buffer_mtx);

    frame_buffer.clear();
    buffer_begin_frame_number = 0;
    buffer_end_frame_number = 0;
    playback_frame_number = -1;
    try
    {
        h_cam.reset(new cv::VideoCapture());
        if (h_cam)
        {
            int index = -1;
            if(!boost::conversion::detail::try_lexical_convert(file_path, index))
            {
                index = -1;
            }

            if (index == -1)
            {
                if (h_cam->open(file_path))
                {
                    loaded_document = file_path;
                    playback_frame_number = 0;
                    return true;
                }
            }
            else
            {
                if (h_cam->open(index))
                {
                    loaded_document = file_path;
                    playback_frame_number = 0;
                    return true;
                }
            }
        }
    }
    catch (cv::Exception& e)
    {
        LOG(debug) << "Unable to load " << file_path << " due to " << e.what();
    }
    return false;
}

long long frame_grabber_cv::GetNumFrames()
{
    if (d_cam)
    {
        return -1;
    }
    if (h_cam)
    {
        return h_cam->get(cv::CAP_PROP_FRAME_COUNT);
    }
    return -1;
}

TS<SyncedMemory> frame_grabber_cv::GetCurrentFrame(cv::cuda::Stream& stream)
{
    return TS<SyncedMemory>(current_frame.timestamp, current_frame.frame_number, current_frame.clone(stream));
}

TS<SyncedMemory> frame_grabber_cv::GetFrameImpl(int index, cv::cuda::Stream& stream)
{
    if (d_cam)
    {

    }
    if (h_cam)
    {
        if (h_cam->set(cv::CAP_PROP_POS_FRAMES, index))
        {
            return GetNextFrameImpl(stream);
        }
    }
    return TS<SyncedMemory>();
}

TS<SyncedMemory> frame_grabber_cv::GetNextFrameImpl(cv::cuda::Stream& stream)
{
    if (d_cam)
    {

    }
    if (h_cam)
    {
        cv::Mat h_mat;
        if (h_cam->read(h_mat))
        {
            if (!h_mat.empty())
            {
                got_frame = true;
                cv::cuda::GpuMat d_mat;
                d_mat.upload(h_mat, stream);
                return TS<SyncedMemory>(h_cam->get(cv::CAP_PROP_POS_MSEC), (long long)h_cam->get(cv::CAP_PROP_POS_FRAMES), h_mat, d_mat);
            }else
            {
                if(got_frame)
                    sig_eos();
            }
        }
        if(got_frame)
            sig_eos();
    }
    return TS<SyncedMemory>();
}


frame_grabber_camera::frame_grabber_camera()
{
    this->_is_stream = true;
}
::std::vector<::std::string> frame_grabber_camera::ListLoadableDocuments()
{
    ::std::vector<::std::string> output;
#ifdef _MSC_VER
    MFStartup(MF_VERSION);
    HRESULT hr = S_OK;
    IMFAttributes *pAttributes = NULL;
    UINT32      m_cDevices; // contains the number of devices
    IMFActivate **m_ppDevices = NULL; // contains properties about each device

                                      // Initialize an attribute store. We will use this to
                                      // specify the enumeration parameters.

    hr = MFCreateAttributes(&pAttributes, 1);

    // Ask for source type = video capture devices
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
            );
    }
    // Enumerate devices.
    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &m_ppDevices, &m_cDevices);
    }
    for (int i = 0; i < m_cDevices; ++i)
    {
        HRESULT hr = S_OK;
        wchar_t* ppszName = nullptr;
        hr = m_ppDevices[i]->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
            &ppszName,
            NULL
            );
        std::wstring wstring(ppszName);
        output.push_back(boost::lexical_cast<std::string>(i) + " - " + std::string(wstring.begin(), wstring.end()));
    }


    SafeRelease(&pAttributes);

    for (UINT32 i = 0; i < m_cDevices; i++)
    {
        SafeRelease(&m_ppDevices[i]);
    }
    CoTaskMemFree(m_ppDevices);
    m_ppDevices = NULL;

    m_cDevices = 0;


#else



#endif
    return output;
}
int frame_grabber_camera::CanLoadDocument(const std::string& doc)
{
    auto pos = doc.find(" - ");
    if(pos != std::string::npos)
    {
        int index = 0;
        if(boost::conversion::detail::try_lexical_convert(doc.substr(pos), index))
        {
            return 10;
        }
    }else
    {
        int index = 0;
        if(boost::conversion::detail::try_lexical_convert(doc, index))
        {
            return 10;
        }
    }
    auto cameras = ListLoadableDocuments();
    for(const auto& camera : cameras)
    {
        if(camera == doc)
            return 10;
    }
    return 0;
}
rcc::shared_ptr<ICoordinateManager> frame_grabber_camera::GetCoordinateManager()
{
    return rcc::shared_ptr<ICoordinateManager>();
}
TS<SyncedMemory> frame_grabber_camera::GetNextFrameImpl(cv::cuda::Stream& stream)
{
    if (d_cam)
    {

    }
    if (h_cam)
    {
        cv::Mat h_mat;
        if (h_cam->read(h_mat))
        {
            if (!h_mat.empty())
            {
                cv::cuda::GpuMat d_mat;
                d_mat.upload(h_mat, stream);
                return TS<SyncedMemory>(0.0, current_timestamp++, h_mat, d_mat);
            }else
            {
                LOG_EVERY_N(warning, 90) << "h_cam->read returned empty frame";
            }
        }else
        {
            LOG_EVERY_N(warning, 90) << "h_cam->read(h_mat) failed";
        }
    }
    if(!d_cam && !h_cam)
    {
        LOG_EVERY_N(warning, 10000) << "no video capture device specified";
    }
    return TS<SyncedMemory>();
}
bool frame_grabber_camera::LoadFile(const std::string& file_path)
{
    int index = 0;
    if(boost::conversion::detail::try_lexical_convert(file_path, index))
    {
        h_cam.reset(new cv::VideoCapture(index));
        return true;
    }else
    {
        index = 0;
    }
    auto cameras = ListLoadableDocuments();
    for(auto camera : cameras)
    {
        if(camera == file_path)
        {
            h_cam.reset(new cv::VideoCapture());
            return h_cam->open(index);
        }
        ++index;
    }
    auto func = [&cameras]() 
    {
        std::stringstream ss; 
        for (auto& cam : cameras) 
            ss << cam << ", "; 
        return ss.str(); 
    };
    LOG(debug) << "Unable to load " << file_path << " queried cameras: " << func() << " trying to requery";
    cameras = ListLoadableDocuments();
    for (auto camera : cameras)
    {
        if (camera == file_path)
        {
            h_cam.reset(new cv::VideoCapture());
            return h_cam->open(index);
        }
        ++index;
    }
    LOG(warning) << "Unable to load " << file_path << " queried cameras: " << func();
    return false;
}
MO_REGISTER_CLASS(frame_grabber_camera);
